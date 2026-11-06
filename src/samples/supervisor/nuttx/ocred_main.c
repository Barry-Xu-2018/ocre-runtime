/**
 * Copyright 2026 Sony Group Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sched.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <ocre/ocre.h>
#include <ocre/shell/shell.h>
#include <ocre/platform/config.h>
#include <ocre/platform/workdir.h>

#include "ocred.h"

/* Containers are threads of the task that starts them. NSH builtin commands
 * are short lived tasks, so the Ocre context and all the containers are owned
 * by this daemon, and the 'ocre' command only forwards its command line here.
 */

static sem_t ready_sem;
static int ready_rc;

static int send_request(struct ocred_request *req)
{
	mqd_t mq = mq_open(OCRED_QUEUE_NAME, O_WRONLY);
	if (mq == (mqd_t)-1) {
		return -1;
	}

	struct ocred_request *p = req;

	int rc = mq_send(mq, (const char *)&p, sizeof(p), 0);
	mq_close(mq);

	return rc;
}

static int ocred_daemon(int argc, char *argv[])
{
	struct ocre_context *ctx = NULL;
	struct mq_attr attr;
	mqd_t mq = (mqd_t)-1;

	if (ocre_initialize(NULL)) {
		fprintf(stderr, "Failed to initialize Ocre\n");
		goto error;
	}

	ctx = ocre_create_context(CONFIG_OCRE_DEFAULT_WORKING_DIRECTORY);
	if (!ctx) {
		fprintf(stderr, "Failed to create Ocre context at '%s'\n", CONFIG_OCRE_DEFAULT_WORKING_DIRECTORY);
		goto error_deinit;
	}

	memset(&attr, 0, sizeof(attr));
	attr.mq_maxmsg = 4;
	attr.mq_msgsize = sizeof(struct ocred_request *);

	mq_unlink(OCRED_QUEUE_NAME);
	mq = mq_open(OCRED_QUEUE_NAME, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
	if (mq == (mqd_t)-1) {
		fprintf(stderr, "Failed to create message queue '%s': errno=%d\n", OCRED_QUEUE_NAME, errno);
		goto error_context;
	}

	fprintf(stderr, "ocred: context at '%s'\n", ocre_context_get_working_directory(ctx));

	ready_rc = 0;
	sem_post(&ready_sem);

	for (;;) {
		struct ocred_request *req;

		if (mq_receive(mq, (char *)&req, sizeof(req), NULL) != sizeof(req)) {
			if (errno == EINTR) {
				continue;
			}

			fprintf(stderr, "ocred: mq_receive failed: errno=%d\n", errno);
			break;
		}

		if (req->argc == 0) {
			req->result = 0;
			sem_post(&req->done);
			break;
		}

		optind = 1;
		req->result = ocre_shell(ctx, req->argc, req->argv);

		fflush(stdout);
		fflush(stderr);

		sem_post(&req->done);
	}

	mq_close(mq);
	mq_unlink(OCRED_QUEUE_NAME);
	ocre_destroy_context(ctx);
	ocre_deinitialize();

	fprintf(stderr, "ocred: stopped\n");

	return 0;

error_context:
	ocre_destroy_context(ctx);
error_deinit:
	ocre_deinitialize();
error:
	ready_rc = -1;
	sem_post(&ready_sem);

	return 1;
}

static int stop_daemon(void)
{
	struct ocred_request req;

	memset(&req, 0, sizeof(req));
	sem_init(&req.done, 0, 0);

	if (send_request(&req)) {
		fprintf(stderr, "ocred is not running\n");
		sem_destroy(&req.done);
		return 1;
	}

	while (sem_wait(&req.done) && errno == EINTR) {
	}

	sem_destroy(&req.done);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc > 1 && !strcmp(argv[1], "stop")) {
		return stop_daemon();
	}

	if (argc > 1) {
		fprintf(stderr, "Usage: %s [stop]\n", argv[0]);
		return 1;
	}

	mqd_t mq = mq_open(OCRED_QUEUE_NAME, O_WRONLY);
	if (mq != (mqd_t)-1) {
		mq_close(mq);
		fprintf(stderr, "ocred is already running\n");
		return 0;
	}

	if (ocre_check_working_directory(CONFIG_OCRE_DEFAULT_WORKING_DIRECTORY)) {
		return 1;
	}

	sem_init(&ready_sem, 0, 0);
	ready_rc = -1;

	int pid = task_create("ocred", CONFIG_OCRE_DAEMON_PRIORITY, CONFIG_OCRE_DAEMON_STACKSIZE, ocred_daemon,
			      NULL);
	if (pid < 0) {
		fprintf(stderr, "Failed to start ocred: errno=%d\n", errno);
		sem_destroy(&ready_sem);
		return 1;
	}

	while (sem_wait(&ready_sem) && errno == EINTR) {
	}

	sem_destroy(&ready_sem);

	if (ready_rc) {
		return 1;
	}

	printf("ocred started (pid %d)\n", pid);

	return 0;
}
