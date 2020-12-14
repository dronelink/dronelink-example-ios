/**
 * Copyright (c) 2017 Parrot Drones SAS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Parrot Drones SAS Company nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE PARROT DRONES SAS COMPANY BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _VBUF_PRIVATE_H_
#define _VBUF_PRIVATE_H_

#include <pthread.h>

#include <futils/futils.h>

#include "vbuf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Forward declaration */
struct vbuf_specific;


/* Buffer object */
struct vbuf_buffer {
	/* Buffer type identifier (must be unique to a buffer implementation) */
	uint32_t type;

	/* Buffer current reference count */
	unsigned int ref_count;

	/* True (not null) when the buffer is write-locked */
	int write_locked;

	/* Node for inclusion in a list */
	struct list_node node;

	/* Originating pool (optional, can be NULL) */
	struct vbuf_pool *pool;

	/* Buffer callback functions */
	struct vbuf_cbs cbs;

	/* Platform-specific data */
	struct vbuf_specific *specific;

	/* Video frame buffer capacity */
	size_t capacity;

	/* Video frame buffer current used size */
	size_t size;

	/* Video frame buffer pointer */
	uint8_t *ptr;

	/* Metadata mutex */
	pthread_mutex_t mutex;

	/* Metadata list */
	struct list_node metas;

	/* User data buffer capacity */
	size_t userdata_capacity;

	/* User data buffer current user size */
	size_t userdata_size;

	/* User data buffer pointer */
	uint8_t *userdata_ptr;
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_VBUF_PRIVATE_H_ */
