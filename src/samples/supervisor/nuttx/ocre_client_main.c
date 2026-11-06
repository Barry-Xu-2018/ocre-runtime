/**
 * Copyright 2026 Sony Group Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

#include "ocred.h"

/* Forwards the command line to 'ocred', which runs it with ocre_shell() */

int main(int argc, char *argv[])
{
	struct ocred_request req;
	struct ocred_request *p = &req;

	mqd_t mq = mq_open(OCRED_QUEUE_NAME, O_WRONLY);
	if (mq == (mqd_t)-1) {
		fprintf(stderr, "ocred is not running. Start it with 'ocred'\n");
		return 1;
	}

	memset(&req, 0, sizeof(req));
	req.argc = argc;
	req.argv = argv;
	sem_init(&req.done, 0, 0);

	if (mq_send(mq, (const char *)&p, sizeof(p), 0)) {
		fprintf(stderr, "Failed to send request to ocred: errno=%d\n", errno);
		mq_close(mq);
		sem_destroy(&req.done);
		return 1;
	}

	mq_close(mq);

	while (sem_wait(&req.done) && errno == EINTR) {
	}

	sem_destroy(&req.done);

	return req.result ? 1 : 0;
}
