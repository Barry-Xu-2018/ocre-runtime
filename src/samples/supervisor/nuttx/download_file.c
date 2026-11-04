#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ocre/platform/config.h>

#ifdef CONFIG_OCRE_IMAGE_PULL

#include <netutils/webclient.h>

#define DOWNLOAD_BUFFER_SIZE 1024

struct download_state {
	int fd;
	size_t total;
};

static int sink_callback(char **buffer, int offset, int datend, int *buflen, void *arg)
{
	struct download_state *state = arg;
	const char *data = *buffer + offset;
	size_t len = datend - offset;

	while (len > 0) {
		ssize_t written = write(state->fd, data, len);
		if (written < 0) {
			if (errno == EINTR) {
				continue;
			}

			fprintf(stderr, "Failed to write image: errno=%d\n", errno);
			return -errno;
		}

		data += written;
		len -= written;
		state->total += written;
	}

	return 0;
}

int ocre_download_file(const char *url, const char *filepath)
{
	struct webclient_context ctx;
	struct download_state state;
	char *buffer;
	int ret;

	if (strncmp(url, "http://", 7)) {
		fprintf(stderr, "Only http:// URLs are supported\n");
		return -1;
	}

	buffer = malloc(DOWNLOAD_BUFFER_SIZE);
	if (!buffer) {
		fprintf(stderr, "Failed to allocate download buffer\n");
		return -1;
	}

	state.total = 0;
	state.fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (state.fd < 0) {
		fprintf(stderr, "Failed to open '%s': errno=%d\n", filepath, errno);
		free(buffer);
		return -1;
	}

	webclient_set_defaults(&ctx);
	ctx.method = "GET";
	ctx.url = url;
	ctx.buffer = buffer;
	ctx.buflen = DOWNLOAD_BUFFER_SIZE;
	ctx.sink_callback = sink_callback;
	ctx.sink_callback_arg = &state;

	ret = webclient_perform(&ctx);

	close(state.fd);
	free(buffer);

	if (ret < 0) {
		fprintf(stderr, "HTTP request failed: %d\n", ret);
		goto error;
	}

	if (ctx.http_status != 200) {
		fprintf(stderr, "HTTP status %u\n", ctx.http_status);
		goto error;
	}

	fprintf(stderr, "Downloaded %zu bytes\n", state.total);

	return 0;

error:
	unlink(filepath);

	return -1;
}

#else /* !CONFIG_OCRE_IMAGE_PULL */

int ocre_download_file(const char *url, const char *filepath)
{
	fprintf(stderr, "Image pull is not enabled (CONFIG_OCRE_IMAGE_PULL)\n");

	return -1;
}

#endif /* CONFIG_OCRE_IMAGE_PULL */
