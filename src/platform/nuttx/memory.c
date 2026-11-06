/**
 * Copyright 2026 Sony Group Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <malloc.h>

void *user_malloc(size_t size)
{
	return memalign(8, size);
}

void user_free(void *ptr)
{
	free(ptr);
}

void *user_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}
