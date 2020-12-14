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

#ifndef _VSTRM_FRAME_H_
#define _VSTRM_FRAME_H_


/* Forward declaration */
struct vstrm_frame;


/* Insert NAL unit start codes during frame copy */
#define VSTRM_FRAME_COPY_FLAGS_INSERT_NALU_START_CODE 0x01

/* Insert NAL unit sizes during frame copy */
#define VSTRM_FRAME_COPY_FLAGS_INSERT_NALU_SIZE 0x02

/* Filter out SPS/PPS NAL units during frame copy */
#define VSTRM_FRAME_COPY_FLAGS_FILTER_SPS_PPS 0x04

/* Filter out SEI NAL units during frame copy */
#define VSTRM_FRAME_COPY_FLAGS_FILTER_SEI 0x08

/* Maximum number of NAL units importance levels */
#define VSTRM_FRAME_MAX_NALU_IMPORTANCE_LEVELS 4

/* Maximum number of NAL units priority levels */
#define VSTRM_FRAME_MAX_NALU_PRIORITY_LEVELS 5


/* Codec */
enum vstrm_codec {
	/* Unknown codec */
	VSTRM_CODEC_UNKNOWN = 0,

	/* H.264/AVC video codec */
	VSTRM_CODEC_VIDEO_H264,
};


/* Macroblock status */
enum vstrm_frame_mb_status {
	/* The macroblock status is unknown */
	VSTRM_FRAME_MB_STATUS_UNKNOWN = 0,

	/* The macroblock is valid and contained in an I-slice */
	VSTRM_FRAME_MB_STATUS_VALID_ISLICE,

	/* The macroblock is valid and contained in a P-slice */
	VSTRM_FRAME_MB_STATUS_VALID_PSLICE,

	/* The macroblock is missing and concealed */
	VSTRM_FRAME_MB_STATUS_MISSING_CONCEALED,

	/* The macroblock is missing and not concealed */
	VSTRM_FRAME_MB_STATUS_MISSING,

	/* The macroblock is valid but within an error propagation */
	VSTRM_FRAME_MB_STATUS_ERROR_PROPAGATION,
};


/* Codec information */
struct vstrm_codec_info {
	/* Codec */
	enum vstrm_codec codec;

	union {
		struct {
			/* Picture width in pixels */
			uint32_t width;

			/* Picture height in pixels */
			uint32_t height;

			/* H.264 SPS (raw NALU without start code) */
			uint8_t sps[64];

			/* H.264 SPS size in bytes */
			uint32_t spslen;

			/* H.264 PPS (raw NALU without start code) */
			uint8_t pps[64];

			/* H.264 PPS size in bytes */
			uint32_t ppslen;
		} h264;
	};
};


/* Frame timestamps */
struct vstrm_frame_timestamps {
	/* NTP timestamp; for a sender, only this timestamp is used and it
	 * is mandatory; this timestamp is monotonic; a 0 value means the
	 * timestamp is not available; on a receiver, when ntp and ntp_unskewed
	 * are not available ntp_raw or ntp_raw_unskewed should be used; */
	uint64_t ntp;

	/* Unskewed NTP timestamp (receiver only); same as ntp but taking
	 * into account the clock skew between the sender and the receiver;
	 * a 0 value means the timestamp is not available */
	uint64_t ntp_unskewed;

	/* Raw NTP timestamp computed from RTP timestamp:
	 * NTP = RTP * 1000000 / clkrate (receiver only); this timestamp is
	 * monotonic and always valid; it can be used for presentation but
	 * cannot be used for synchronization between multiple streams */
	uint64_t ntp_raw;

	/* Unskewed raw NTP timestamp (receiver only); same as ntp_raw but
	 * taking into account the clock skew between the sender and the
	 * receiver; this timestamp is always valid */
	uint64_t ntp_raw_unskewed;

	/* Local timestamp (receiver only); this timestamp based on the
	 * local monotonic clock is computed from the clock delta algorithm
	 * with a precision of round-trip-delay / 2; a 0 value means the
	 * timestamp is not available; this timestamp should be used only
	 * for statistics or debugging purposes and should not be used for
	 * presentation */
	uint64_t local;

	/* First packet reception timestamp on the local monotonic clock */
	uint64_t recv;
};


/* Frame information */
/* clang-format off */
struct vstrm_frame_info {
	/* Frame is syntactically complete */
	uint32_t complete:1;

	/* Frame has errors (either missing slices or error propagation
	 * from a missing slice in previous reference frames);
	 * note: the frame is always syntactically correct, meaning that
	 * frames can have missing NAL units but present NAL units are
	 * decodable without errors */
	uint32_t error:1;

	/* Frame is reference */
	uint32_t ref:1;

	/* Frame is a generated grey IDR frame */
	uint32_t gen_grey_idr:1;

	/* H.264 frame width in macroblocks */
	uint32_t mb_width;

	/* H.264 frame height in macroblocks */
	uint32_t mb_height;

	/* H.264 frame total macroblocks (mb_width * mb_height) */
	uint32_t mb_total;

	/* H.264 frame macroblock status (see vstrm_frame_mb_status) */
	uint8_t *mb_status;
};
/* clang-format on */


/* Operations associated with a frame */
struct vstrm_frame_ops {
	/* Called when the last reference on the frame is released.
	 * @param base: pointer to the video frame structure */
	void (*dispose)(struct vstrm_frame *base);
};


/* NAL unit of a frame with its data; this is a single H.264 raw NAL unit
 * without start code */
struct vstrm_frame_nalu {
	/* NAL unit data; its lifetime is at least the one of the frame */
	const uint8_t *cdata;

	/* NAL unit size in bytes */
	size_t len;

	/* Priority of the NAL unit (sender only);
	 * (0 .. VSTRM_FRAME_MAX_NALU_PRIORITY_LEVELS) */
	uint32_t priority;

	/* Importance of the NAL unit (sender only);
	 * (0 .. VSTRM_FRAME_MAX_NALU_IMPORTANCE_LEVELS) */
	uint32_t importance;

	/* Additional user data associated with this NAL unit */
	void *userdata;
};


/* Video frame */
struct vstrm_frame {
	/* Associated operations */
	struct vstrm_frame_ops ops;

	/* Internal reference count */
	uint32_t refcount;

	/* Timestamps */
	struct vstrm_frame_timestamps timestamps;

	/* NAL units of the frame */
	struct vstrm_frame_nalu *nalus;

	/* Number of NAL units */
	uint32_t nalu_count;

	/* Maximum number of allocated NAL units */
	uint32_t max_nalu_count;

	/* Metadata of the frame */
	struct vmeta_frame metadata;

	/* More information about the frame (receiver only) */
	struct vstrm_frame_info info;

	/* Video statistics - static (receiver only) */
	struct vstrm_video_stats video_stats;

	/* Video statistics - dynamic (receiver only) */
	struct vstrm_video_stats_dyn video_stats_dyn;

	/* To be used in a list */
	struct list_node node;

	/* Additional user data associated with this frame */
	void *userdata;
};


/**
 * Create a new empty frame structure.
 * @param ops: operations to associate with the frame
 * @param extra_size: allocate extra memory at the end of the structure and
 *                    store the pointer in the frame userdata
 * @param ret_obj: pointer to returned frame structure (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_frame_new(const struct vstrm_frame_ops *ops,
		    size_t extra_size,
		    struct vstrm_frame **ret_obj);


/**
 * Increase the reference count of the frame.
 * @param self: pointer to the frame structure
 */
VSTRM_API
void vstrm_frame_ref(struct vstrm_frame *self);


/**
 * Decrease the reference count of the frame.
 * When it reaches 0, the dispose callback is called and the frame structure
 * is freed. It is up to the dispose callback to free internal data stored
 * in NAL units. Only the frame structure itself is freed.
 * @param self: pointer to the frame structure
 */
VSTRM_API
void vstrm_frame_unref(struct vstrm_frame *self);


/**
 * Add a NAL unit to the frame.
 * The lifetime of the NAL unit must be at least the one of the frame.
 * @param self: pointer to the frame structure
 * @param nalu: pointer to the NALU to add; no copy of the data will be done,
 *              only pointers will be copied in the frame
 * @return 0 on success, negative errno value in case of error; -EPERM is
 *         returned if the frame is shared
 */
VSTRM_API
int vstrm_frame_add_nalu(struct vstrm_frame *self,
			 const struct vstrm_frame_nalu *nalu);


/**
 * Get the total size of the data of the frame.
 * @param self: pointer to the frame structure
 * @param size: pointer to the frame size in bytes (output)
 * @param flags: optional flags, see VSTRM_FRAME_COPY_FLAGS_xxx
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_frame_get_size(struct vstrm_frame *self,
			 size_t *size,
			 uint32_t flags);


/**
 * Copy the contents of the frame.
 * @param self: pointer to the frame structure
 * @param buf: destination buffer
 * @param len: size in bytes of the destination buffer, must be at least
 *             the size returned by vstrm_frame_get_size()
 * @param flags: optional flags, see VSTRM_FRAME_COPY_FLAGS_xxx
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_frame_copy(struct vstrm_frame *self,
		     uint8_t *buf,
		     size_t len,
		     uint32_t flags);


/**
 * ToString function for enum vstrm_codec.
 * @param val: codec value to convert
 * @return a string description of the codec
 */
VSTRM_API
const char *vstrm_codec_str(enum vstrm_codec val);


/**
 * ToString function for enum vstrm_frame_mb_status.
 * @param val: macroblock status value to convert
 * @return a string description of the macroblock status
 */
VSTRM_API
const char *vstrm_frame_mb_status_str(enum vstrm_frame_mb_status val);


#endif /* !_VSTRM_FRAME_H_ */
