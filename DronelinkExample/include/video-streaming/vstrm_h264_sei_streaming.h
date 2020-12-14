/**
 * Copyright (c) 2016 Parrot Drones SAS
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

#ifndef _VSTRM_H264_SEI_STREAMING_H_
#define _VSTRM_H264_SEI_STREAMING_H_


/* "Parrot Streaming" v1 user data SEI UUID;
 * UUID: 13dbccc7-c720-42f5-a0b7-aafaa2b3af97 */
#define VSTRM_H264_SEI_STREAMING_V1_UUID_0 0x13dbccc7
#define VSTRM_H264_SEI_STREAMING_V1_UUID_1 0xc72042f5
#define VSTRM_H264_SEI_STREAMING_V1_UUID_2 0xa0b7aafa
#define VSTRM_H264_SEI_STREAMING_V1_UUID_3 0xa2b3af97


/* "Parrot Streaming" v1 parameters */
struct vstrm_h264_sei_streaming_v1 {
	/* Frame index in GOP */
	uint8_t index_in_gop;

	/* Number of slices per frame */
	uint8_t slice_count;

	/* Number of macroblocks in each slice */
	uint16_t slice_mb_count[256];
};


/* "Parrot Streaming" v2 user data SEI UUID;
 * UUID: e5cedca1-86b7-4254-9601-434fffcd1f56 */
#define VSTRM_H264_SEI_STREAMING_V2_UUID_0 0xe5cedca1
#define VSTRM_H264_SEI_STREAMING_V2_UUID_1 0x86b74254
#define VSTRM_H264_SEI_STREAMING_V2_UUID_2 0x9601434f
#define VSTRM_H264_SEI_STREAMING_V2_UUID_3 0xffcd1f56


/* "Parrot Streaming" v2 parameters */
struct vstrm_h264_sei_streaming_v2 {
	/* Number of slices per frame */
	uint16_t slice_count;

	/* Number of macroblocks per slice (the last slice
	 * of the frame may be smaller) */
	uint16_t slice_mb_count;
};


/**
 * Test if the input UUID is the one of "Parrot Streaming" v1 user data SEI.
 * @param uuid: user data SEI UUID
 * @return 1 if the UUID is the one of "Parrot Streaming" v1 user data SEI
 *         or 0 otherwise, negative errno value in case of error
 */
VSTRM_API
int vstrm_h264_sei_streaming_is_v1(const uint8_t uuid[16]);


/**
 * Get the required size in bytes for serializing a "Parrot Streaming" v1
 * user data SEI (excluding the UUID).
 * The SEI structure must have been previously filled.
 * @param sei: pointer to a "Parrot Streaming" v1 user data SEI structure
 * @return the size in bytes of a "Parrot Streaming" v1 user data SEI,
 *         negative errno value in case of error
 */
VSTRM_API
ssize_t vstrm_h264_sei_streaming_v1_get_size(
	const struct vstrm_h264_sei_streaming_v1 *sei);


/**
 * Write "Parrot Streaming" v1 user data SEI.
 * This function writes the "Parrot Streaming" v1 UUID in the uuid array and
 * fills the supplied buffer with the "Parrot Streaming" v1 user data SEI
 * (without the UUID). The size of the data written is returned through the
 * len parameter.
 * The buffer must have been previously allocated and its size must be
 * supplied through the len parameter. The ownership of the buffer stays with
 * the caller.
 * @param sei: pointer to a "Parrot Streaming" v1 user data SEI structure
 * @param uuid: pointer to the UUID array (output)
 * @param buf: pointer to the buffer to write to (output)
 * @param len: pointer to the buffer length (input/output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_h264_sei_streaming_v1_write(
	const struct vstrm_h264_sei_streaming_v1 *sei,
	uint8_t uuid[16],
	uint8_t *buf,
	size_t *len);


/**
 * Read "Parrot Streaming" v1 user data SEI.
 * This function checks the UUID value and fills the supplied structure with
 * the deserialized "Parrot Streaming" v1 user data SEI.
 * The size of the buffer (len parameter) must be sufficient to allow reading
 * the data, otherwise an error is returned. The ownership of the buffer stays
 * with the caller.
 * @param sei: pointer to a "Parrot Streaming" v1 user data SEI
 *             structure (output)
 * @param uuid: pointer to the UUID array
 * @param buf: pointer to the buffer to read from
 * @param len: buffer length
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_h264_sei_streaming_v1_read(struct vstrm_h264_sei_streaming_v1 *sei,
				     const uint8_t uuid[16],
				     const uint8_t *buf,
				     size_t len);


/**
 * Test if the input UUID is the one of "Parrot Streaming" v2 user data SEI.
 * @param uuid: user data SEI UUID
 * @return 1 if the UUID is the one of "Parrot Streaming" v2 user data SEI
 *         or 0 otherwise, negative errno value in case of error
 */
VSTRM_API
int vstrm_h264_sei_streaming_is_v2(const uint8_t uuid[16]);


/**
 * Get the required size in bytes for serializing a "Parrot Streaming" v2
 * user data SEI (excluding the UUID).
 * The SEI structure must have been previously filled.
 * @param sei: pointer to a "Parrot Streaming" v2 user data SEI structure
 * @return the size in bytes of a "Parrot Streaming" v2 user data SEI,
 *         negative errno value in case of error
 */
VSTRM_API
ssize_t vstrm_h264_sei_streaming_v2_get_size(
	const struct vstrm_h264_sei_streaming_v2 *sei);


/**
 * Write "Parrot Streaming" v2 user data SEI.
 * This function writes the "Parrot Streaming" v2 UUID in the uuid array and
 * fills the supplied buffer with the "Parrot Streaming" v2 user data SEI
 * (without the UUID). The size of the data written is returned through the
 * len parameter.
 * The buffer must have been previously allocated and its size must be
 * supplied through the len parameter. The ownership of the buffer stays with
 * the caller.
 * @param sei: pointer to a "Parrot Streaming" v2 user data SEI structure
 * @param uuid: pointer to the UUID array (output)
 * @param buf: pointer to the buffer to write to (output)
 * @param len: pointer to the buffer length (input/output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_h264_sei_streaming_v2_write(
	const struct vstrm_h264_sei_streaming_v2 *sei,
	uint8_t uuid[16],
	uint8_t *buf,
	size_t *len);


/**
 * Read "Parrot Streaming" v2 user data SEI.
 * This function checks the UUID value and fills the supplied structure with
 * the deserialized "Parrot Streaming" v2 user data SEI.
 * The size of the buffer (len parameter) must be sufficient to allow reading
 * the data, otherwise an error is returned. The ownership of the buffer stays
 * with the caller.
 * @param sei: pointer to a "Parrot Streaming" v2 user data SEI
 *             structure (output)
 * @param uuid: pointer to the UUID array
 * @param buf: pointer to the buffer to read from
 * @param len: buffer length
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_h264_sei_streaming_v2_read(struct vstrm_h264_sei_streaming_v2 *sei,
				     const uint8_t uuid[16],
				     const uint8_t *buf,
				     size_t len);


#endif /* !_VSTRM_H264_SEI_STREAMING_H_ */
