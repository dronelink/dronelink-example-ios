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

#ifndef _VBUF_H_
#define _VBUF_H_

#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* To be used for all public API */
#ifdef VBUF_API_EXPORTS
#	ifdef _WIN32
#		define VBUF_API __declspec(dllexport)
#	else /* !_WIN32 */
#		define VBUF_API __attribute__((visibility("default")))
#	endif /* !_WIN32 */
#else /* !VBUF_API_EXPORTS */
#	define VBUF_API
#endif /* !VBUF_API_EXPORTS */


/* Forward declarations */
struct vbuf_buffer;
struct vbuf_pool;
struct vbuf_queue;


/* Buffer callback functions */
struct vbuf_cbs {
	/* Allocation callback function (mandatory) called on buffer creation
	 * after the buffer object has been created and the internal members
	 * are initialized (capacity, pool...). This function performs the
	 * actual buffer memory allocation or mapping.
	 * @param buf: pointer on a buffer object
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*alloc)(struct vbuf_buffer *buf, void *userdata);

	/* Allocation callback function user data pointer */
	void *alloc_userdata;

	/* Reallocation callback function (optional) called when reallocating
	 * the buffer is required. If this function is not defined the
	 * reallocation will fail.
	 * @param buf: pointer on a buffer object
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*realloc)(struct vbuf_buffer *buf, void *userdata);

	/* Reallocation callback function user data pointer */
	void *realloc_userdata;

	/* Unreference callback function (optional) called when last
	 * unreferencing a buffer and the reference count drops to 0,
	 * before either destroying the buffer or returning it to its
	 * originating pool.
	 * @param buf: pointer on a buffer object
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*unref)(struct vbuf_buffer *buf, void *userdata);

	/* Unreference callback function user data pointer */
	void *unref_userdata;

	/* Free callback function (mandatory) called on buffer destruction
	 * before the buffer object is destroyed. This function performs the
	 * actual buffer memory freeing or unmapping.
	 * @param buf: pointer on a buffer object
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*free)(struct vbuf_buffer *buf, void *userdata);

	/* Free callback function user data pointer */
	void *free_userdata;

	/* Pool get callback function (optional) called when a buffer is
	 * obtained from the originating pool (if the buffer belongs to a pool).
	 * @param buf: pointer on a buffer object
	 * @param timeout_ms: timeout in milliseconds (0 means no wait,
	 *                    negative value means wait forever)
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*pool_get)(struct vbuf_buffer *buf,
			int timeout_ms,
			void *userdata);

	/* Pool get callback function user data pointer */
	void *pool_get_userdata;

	/* Pool put callback function (optional) called before a buffer is
	 * returned to the originating pool (if the buffer belongs to a pool).
	 * @param buf: pointer on a buffer object
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*pool_put)(struct vbuf_buffer *buf, void *userdata);

	/* Pool put callback function user data pointer */
	void *pool_put_userdata;

	/* Queue push callback function (optional) called before a buffer is
	 * pushed into a queue.
	 * @param buf: pointer on a buffer object
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*queue_push)(struct vbuf_buffer *buf, void *userdata);

	/* Queue push callback function user data pointer */
	void *queue_push_userdata;

	/* Queue peek callback function (optional) called when a buffer is
	 * peeked at in a queue.
	 * @param buf: pointer on a buffer object
	 * @param timeout_ms: timeout in milliseconds (0 means no wait,
	 *                    negative value means wait forever)
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*queue_peek)(struct vbuf_buffer *buf,
			  int timeout_ms,
			  void *userdata);

	/* Queue peek callback function user data pointer */
	void *queue_peek_userdata;

	/* Queue pop callback function (optional) called when a buffer is
	 * output from a queue.
	 * @param buf: pointer on a buffer object
	 * @param timeout_ms: timeout in milliseconds (0 means no wait,
	 *                    negative value means wait forever)
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*queue_pop)(struct vbuf_buffer *buf,
			 int timeout_ms,
			 void *userdata);

	/* Queue pop callback function user data pointer */
	void *queue_pop_userdata;
};


/**
 * Buffer API
 */

/**
 * Create a buffer.
 * At least the alloc and free callbacks must be defined.
 * When no longer needed, the buffer must be unreferenced using the
 * vbuf_unref() function. When a buffer is no longer referenced it is either
 * freed or returned to its originating pool (if the parameter is set).
 * The created buffer object is returned through the ret_obj parameter.
 * @param capacity: buffer capacity (can be 0 and reallocated later)
 * @param userdata_capacity: user data buffer capacity (can be 0)
 * @param cbs: buffer callback functions and user data
 * @param pool: optional buffer pool (can be NULL)
 * @param ret_obj: pointer to the created buffer object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_new(size_t capacity,
		      size_t userdata_capacity,
		      const struct vbuf_cbs *cbs,
		      struct vbuf_pool *pool,
		      struct vbuf_buffer **ret_obj);


/**
 * Reference a buffer.
 * This function increments the reference counter of a buffer.
 * @param buf: pointer on a buffer object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_ref(struct vbuf_buffer *buf);


/**
 * Unreference a buffer.
 * This function decrements the reference counter of a buffer.
 * If the buffer's reference counter reaches zero, it is either returned
 * to its originating pool (if defined) or freed.
 * @param buf: pointer on a buffer object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_unref(struct vbuf_buffer *buf);


/**
 * Get the buffer's reference count.
 * This function returns the current value of the reference counter.
 * @param buf: pointer on a buffer object
 * @return the reference count on success, negative errno value in case of error
 */
VBUF_API int vbuf_get_ref_count(struct vbuf_buffer *buf);


/**
 * Lock a buffer for writing.
 * This function sets a buffer to read only. With a locked buffer the
 * vbuf_get_data() function cannot be used, only vbuf_get_cdata() can be used.
 * Locking a buffer is only available when its reference counter is 1.
 * @param buf: pointer on a buffer object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_write_lock(struct vbuf_buffer *buf);


/**
 * Unlock a buffer for writing.
 * This function sets a buffer to read/write. With an unlocked buffer both the
 * vbuf_get_data() and vbuf_get_cdata() functions can be used.
 * Unlocking a buffer is only available when its reference counter is 1.
 * @param buf: pointer on a buffer object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_write_unlock(struct vbuf_buffer *buf);


/**
 * Get the write lock status.
 * @param buf: pointer on a buffer object
 * @return 0 if unlocked, 1 if locked, or negative errno value in case of error
 */
VBUF_API int vbuf_is_write_locked(struct vbuf_buffer *buf);


/**
 * Get the buffer's originating pool.
 * If the buffer is not originating from a pool, NULL is returned.
 * @param buf: pointer on a buffer object
 * @return the originating pool pointer on success, NULL in case of error
 */
VBUF_API struct vbuf_pool *vbuf_get_pool(struct vbuf_buffer *buf);


/**
 * Get the buffer's data pointer (read/write).
 * This function fails if the buffer is write-locked.
 * @param buf: pointer on a buffer object
 * @return the data pointer on success, NULL in case of error
 */
VBUF_API uint8_t *vbuf_get_data(struct vbuf_buffer *buf);


/**
 * Get the buffer's data pointer (read only).
 * @param buf: pointer on a buffer object
 * @return the data pointer on success, NULL in case of error
 */
VBUF_API const uint8_t *vbuf_get_cdata(struct vbuf_buffer *buf);


/**
 * Get the buffer currently allocated capacity.
 * @param buf: pointer on a buffer object
 * @return the buffer capacity on success, negative errno value in case of error
 */
VBUF_API ssize_t vbuf_get_capacity(struct vbuf_buffer *buf);


/**
 * Set the buffer allocated capacity.
 * This function reallocates the buffer if necessary. If reallocation is not
 * supported in the underlying implementation the function fails with an error.
 * @param buf: pointer on a buffer object
 * @param capacity: new buffer capacity
 * @return the new buffer capacity on success, negative errno value
 *         in case of error
 */
VBUF_API ssize_t vbuf_set_capacity(struct vbuf_buffer *buf, size_t capacity);


/**
 * Get the buffer currently used size.
 * @param buf: pointer on a buffer object
 * @return the buffer size on success, negative errno value in case of error
 */
VBUF_API ssize_t vbuf_get_size(struct vbuf_buffer *buf);


/**
 * Set the buffer used size.
 * The size must be less than or equal to the current buffer capacity.
 * @param buf: pointer on a buffer object
 * @param size: new buffer size
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_set_size(struct vbuf_buffer *buf, size_t size);


/**
 * Copy a buffer.
 * This function copies the buffer data along with the metadata and user data
 * if available. The destination buffer will be reallocated to an increased
 * capacity if needed.
 * @param src_buf: pointer on the source buffer object
 * @param dst_buf: pointer on the destination buffer object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_copy(struct vbuf_buffer *src_buf,
		       struct vbuf_buffer *dst_buf);


/**
 * Get the buffer's user data pointer (read/write).
 * If no user data buffer is allocated yet, NULL is returned.
 * A user data buffer can be allocated using the vbuf_set_userdata_capacity()
 * function.
 * @param buf: pointer on a buffer object
 * @return the data pointer on success, NULL in case of error
 */
VBUF_API uint8_t *vbuf_get_userdata(struct vbuf_buffer *buf);


/**
 * Get the buffer's user data pointer (read only).
 * If no user data buffer is allocated yet, NULL is returned.
 * A user data buffer can be allocated using the vbuf_set_userdata_capacity()
 * function.
 * @param buf: pointer on a buffer object
 * @return the data pointer on success, NULL in case of error
 */
VBUF_API const uint8_t *vbuf_get_cuserdata(struct vbuf_buffer *buf);


/**
 * Get the buffer user data currently allocated capacity.
 * @param buf: pointer on a buffer object
 * @return the user data capacity on success, negative errno value in case
 *         of error
 */
VBUF_API ssize_t vbuf_get_userdata_capacity(struct vbuf_buffer *buf);


/**
 * Set the buffer user data allocated capacity.
 * This function reallocates the buffer user data if necessary.
 * @param buf: pointer on a buffer object
 * @param capacity: new user data capacity
 * @return the new user data buffer capacity on success, negative errno
 *         value in case of error
 */
VBUF_API ssize_t vbuf_set_userdata_capacity(struct vbuf_buffer *buf,
					    size_t capacity);


/**
 * Get the buffer user data currently used size.
 * @param buf: pointer on a buffer object
 * @return the user data size on success, negative errno value in case of error
 */
VBUF_API ssize_t vbuf_get_userdata_size(struct vbuf_buffer *buf);


/**
 * Set the buffer user data used size.
 * The size must be less than or equal to the current user data capacity.
 * @param buf: pointer on a buffer object
 * @param size: new user data size
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_set_userdata_size(struct vbuf_buffer *buf, size_t size);


/**
 * Copy a buffer's user data.
 * This function copies a source buffer user data to a destination buffer
 * user data. The destination buffer user data will be reallocated to an
 * increased capacity if needed.
 * @param src_buf: pointer on the source buffer object
 * @param dst_buf: pointer on the destination buffer object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_userdata_copy(struct vbuf_buffer *src_buf,
				struct vbuf_buffer *dst_buf);


/**
 * Add metadata to a buffer.
 * This function allocates metadata of size len associated with the buffer.
 * The metadata is associated to a key (which must be unique within a given
 * buffer) and a level. Null level is authorized but discouraged as it cannot
 * be used with other metadata of non-null levels. Levels are useful with the
 * vbuf_metadata_copy() function which can copy metadata only up to a given
 * level (excluded).
 * @param buf: pointer on a buffer object
 * @param key: metadata key
 * @param level: metadata level
 * @param len: metadata buffer size
 * @param ret_ptr: pointer to the allocated metadata (output)
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_metadata_add(struct vbuf_buffer *buf,
			       void *key,
			       unsigned int level,
			       size_t len,
			       uint8_t **ret_ptr);


/**
 * Get the metadata from a buffer.
 * This function gets the metadata associated with a key and optionally
 * outputs the metadata level and size.
 * @param buf: pointer on a buffer object
 * @param key: metadata key
 * @param level: optional pointer on the metadata level (output)
 * @param len: optional pointer on the metadata size (output)
 * @param ret_ptr: pointer to the metadata (output)
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_metadata_get(struct vbuf_buffer *buf,
			       void *key,
			       unsigned int *level,
			       size_t *len,
			       uint8_t **ret_ptr);


/**
 * Remove the metadata from a buffer.
 * This function removes the metadata associated with a key and frees the
 * used memory.
 * @param buf: pointer on a buffer object
 * @param key: metadata key
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_metadata_remove(struct vbuf_buffer *buf, void *key);


/**
 * Copy a buffer's metadata.
 * This function copies a source buffer metadata to a destination buffer
 * metadata. If the max_level parameter is 0, all metadata from the source
 * buffer is copied to the destination buffer, otherwise only the metadata
 * with a level up to max_level (excluded) is copied.
 * @param src_buf: pointer on the source buffer object
 * @param dst_buf: pointer on the destination buffer object
 * @param max_level: maximum level (excluded) of metadata to copy
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_metadata_copy(struct vbuf_buffer *src_buf,
				struct vbuf_buffer *dst_buf,
				unsigned int max_level);


/**
 * Pool API
 */

/**
 * Create a buffer pool.
 * The pool buffer count is mandatory and cannot be updated later.
 * The capacity and userdata_capacity parameters are optional and can be 0,
 * then the memory can be reallocated later if realloc is supported in the
 * underlying buffer implementation. At least the alloc and free callbacks
 * must be defined.
 * When no longer needed, the pool must be freed using the vbuf_pool_destroy()
 * function.
 * The created buffer pool object is returned through the ret_obj parameter.
 * @param count: buffer count
 * @param capacity: individual buffer capacity (can be 0 and reallocated later)
 * @param userdata_capacity: individual user data buffer capacity (can be 0)
 * @param cbs: buffer callback functions and user data
 * @param ret_obj: pointer to the created buffer pool object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_pool_new(unsigned int count,
			   size_t capacity,
			   size_t userdata_capacity,
			   const struct vbuf_cbs *cbs,
			   struct vbuf_pool **ret_obj);


/**
 * Destroy a buffer pool.
 * This function destroys a buffer pool and frees the associated buffers.
 * All buffers should have been previoulsy unreferenced and returned to
 * the pool.
 * @param pool: pointer on a buffer pool object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_pool_destroy(struct vbuf_pool *pool);


/**
 * Get the pool buffer count.
 * This function returns the current number of buffers available in the pool.
 * @param pool: pointer on a buffer pool object
 * @return the buffer count on success, negative errno value in case of error
 */
VBUF_API int vbuf_pool_get_count(struct vbuf_pool *pool);


/**
 * Get a buffer from the pool.
 * This function outputs a buffer from the pool, increasing its reference
 * count to 1.
 * If no buffer is currently available, the function waits up to timeout_ms
 * milliseconds for a buffer to become available. If timeout_ms is 0, the
 * function returns immediately with a -EAGAIN error. If waiting timed out
 * and still no buffer is available, a -ETIMEDOUT error is returned. If
 * timeout_ms is negative, the function waits forever for a buffer to become
 * available (or until vbuf_pool_abort() is called).
 * On success the buffer is returned in the value pointed by the buf parameter.
 * @param pool: pointer on a buffer pool object
 * @param timeout_ms: timeout in milliseconds (0 means no wait,
 *                    negative value means wait forever)
 * @param buf: pointer on a buffer object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int
vbuf_pool_get(struct vbuf_pool *pool, int timeout_ms, struct vbuf_buffer **buf);


/**
 * Abort waiting for a buffer.
 * This function aborts any wait in progress in a vbuf_pool_get() call,
 * which will return a -EAGAIN error.
 * @param pool: pointer on a buffer pool object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_pool_abort(struct vbuf_pool *pool);


/**
 * Get the event associated to a buffer pool.
 * This function returns the pomp_evt associated with the pool.
 * This is useful to be notified in a pomp_loop that a buffer is available
 * in the pool.
 * @param pool: pointer on a buffer pool object
 * @return a pointer on the pomp_evt object on success, NULL in case of error
 */
VBUF_API struct pomp_evt *vbuf_pool_get_evt(struct vbuf_pool *pool);


/**
 * Queue API
 */

/**
 * Create a FIFO buffer queue.
 * The max_count parameter is optional and defines the maximum number of buffer
 * that can be queued. When this parameter is not null and the queue is full,
 * the vbuf_queue_push() function either fails with a -EAGAIN status if
 * the drop_when_full parameter is 0, or succeeds after droping the oldest
 * buffer in the queue otherwise.
 * When no longer needed, the queue must be freed using the
 * vbuf_queue_destroy() function.
 * The created buffer queue object is returned through the ret_obj parameter.
 * @param max_count: maximum buffer count in the queue (optional, can be 0,
 *                   in that case the drop_when_full parameter is ignored)
 * @param drop_when_full: when not null, drop the oldest buffer when pushing
 *                        a buffer if the queue is already full
 * @param ret_obj: pointer to the created buffer queue object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_queue_new(unsigned int max_count,
			    int drop_when_full,
			    struct vbuf_queue **ret_obj);


/**
 * Destroy a buffer queue.
 * This function destroys a buffer queue. All buffers should have been removed
 * from the queue by the application before destroying it. However if the queue
 * is not empty before destruction, a warning log is issued and a flush is
 * performed.
 * @param queue: pointer on a buffer queue object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_queue_destroy(struct vbuf_queue *queue);


/**
 * Get the queue buffer count.
 * This function returns the current number of buffers available in the queue.
 * @param queue: pointer on a buffer queue object
 * @return the buffer count on success, negative errno value in case of error
 */
VBUF_API int vbuf_queue_get_count(struct vbuf_queue *queue);


/**
 * Peek at a buffer in the queue without outputing it.
 * This function return a reference on a buffer in the queue without removing
 * it from the queue. The reference count is unchanged and the buffer must not
 * be unreferenced once no longer needed.
 * The index parameter defines the rank of the buffer: 0 means the next
 * (and oldest) buffer to be output, if index is equal to the value returned
 * by vbuf_queue_get_count() minus 1 it means the most recently pushed buffer.
 * If no buffer is currently available, the function waits up to timeout_ms
 * milliseconds for a buffer to become available. If timeout_ms is 0, the
 * function returns immediately with a -EAGAIN error. If waiting timed out
 * and still no buffer is available, a -ETIMEDOUT error is returned. If
 * timeout_ms is negative, the function waits forever for a buffer to become
 * available (or until vbuf_queue_abort() is called).
 * On success the buffer is returned in the value pointed by the buf parameter.
 * @param queue: pointer on a buffer queue object
 * @param index: buffer index in the queue (lower values mean oldest buffer)
 * @param timeout_ms: timeout in milliseconds (0 means no wait,
 *                    negative value means wait forever)
 * @param buf: pointer on a buffer object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_queue_peek(struct vbuf_queue *queue,
			     unsigned int index,
			     int timeout_ms,
			     struct vbuf_buffer **buf);


/**
 * Get a buffer from the queue.
 * This function outputs a buffer from the queue. The reference count is
 * unchanged, but since the reference count was incremented by 1 when the
 * buffer was pushed in the queue, the buffer must be unreferenced once no
 * longer needed.
 * If no buffer is currently available, the function waits up to timeout_ms
 * milliseconds for a buffer to become available. If timeout_ms is 0, the
 * function returns immediately with a -EAGAIN error. If waiting timed out
 * and still no buffer is available, a -ETIMEDOUT error is returned. If
 * timeout_ms is negative, the function waits forever for a buffer to become
 * available (or until vbuf_queue_abort() is called).
 * On success the buffer is returned in the value pointed by the buf parameter.
 * @param queue: pointer on a buffer queue object
 * @param timeout_ms: timeout in milliseconds (0 means no wait,
 *                    negative value means wait forever)
 * @param buf: pointer on a buffer object pointer (output)
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_queue_pop(struct vbuf_queue *queue,
			    int timeout_ms,
			    struct vbuf_buffer **buf);


/**
 * Push a buffer into the queue.
 * This function pushes a buffer into the queue. The reference count is
 * incremented by 1.
 * If the queue was created with a non-null max_count parameter and the queue
 * is full, the function either fails with a -EAGAIN status if the
 * drop_when_full parameter was 0, or succeeds after droping the oldest
 * buffer in the queue otherwise.
 * @param queue: pointer on a buffer queue object
 * @param buf: pointer on a buffer object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_queue_push(struct vbuf_queue *queue, struct vbuf_buffer *buf);


/**
 * Abort waiting for a buffer.
 * This function aborts any wait in progress in a vbuf_queue_peek() or
 * vbuf_queue_pop() call, which will return a -EAGAIN error.
 * @param queue: pointer on a buffer queue object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_queue_abort(struct vbuf_queue *queue);


/**
 * Flush a buffer queue.
 * This function removes and unreferences all buffers in the queue.
 * @param queue: pointer on a buffer queue object
 * @return 0 on success, negative errno value in case of error
 */
VBUF_API int vbuf_queue_flush(struct vbuf_queue *queue);


/**
 * Get the event associated to a buffer queue.
 * This function returns the pomp_evt associated with the queue.
 * This is useful to be notified in a pomp_loop that a buffer is available
 * in the queue.
 * @param queue: pointer on a buffer queue object
 * @return a pointer on the pomp_evt object on success, NULL in case of error
 */
VBUF_API struct pomp_evt *vbuf_queue_get_evt(struct vbuf_queue *queue);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_VBUF_H_ */
