#ifndef OCRED_H
#define OCRED_H

#include <semaphore.h>

/* Message queue used by the 'ocre' command to forward requests to 'ocred'.
 *
 * Each message is a pointer to a struct ocred_request owned by the client,
 * which stays valid until the daemon posts 'done'. This relies on both tasks
 * sharing the same address space (NuttX FLAT and PROTECTED builds).
 */

#define OCRED_QUEUE_NAME "/ocred"

struct ocred_request {
	int argc;      /* 0 requests the daemon to exit */
	char **argv;
	int result;
	sem_t done;
};

#endif /* OCRED_H */
