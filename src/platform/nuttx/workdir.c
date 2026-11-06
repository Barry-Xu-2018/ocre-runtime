/**
 * Copyright 2026 Sony Group Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statfs.h>

#include <ocre/platform/workdir.h>

int ocre_check_working_directory(const char *path)
{
	struct statfs buf;
	char *p;
	int rc;

	p = strdup(path);
	if (!p) {
		fprintf(stderr, "Failed to allocate memory for path '%s'\n", path);
		return -1;
	}

	/* Find the closest existing parent of the working directory */

	while ((rc = statfs(p, &buf)) && errno == ENOENT && strcmp(p, "/")) {
		char *slash = strrchr(p, '/');
		if (!slash || slash == p) {
			strcpy(p, "/");
		} else {
			*slash = '\0';
		}
	}

	if (rc) {
		fprintf(stderr, "Failed to check working directory '%s': errno=%d\n", path, errno);
		free(p);
		return -1;
	}

	free(p);

	/* The pseudo file system reports itself as procfs */

	if (buf.f_type == PROC_SUPER_MAGIC) {
		fprintf(stderr,
			"Working directory '%s' is not on a mounted file system.\n"
			"Is the SD card inserted and formatted as FAT? Check with 'mount'.\n",
			path);
		return -1;
	}

	return 0;
}
