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

#ifndef _VSTRM_VIDEO_STATS_H_
#define _VSTRM_VIDEO_STATS_H_


/* Video statistics version number */
#define VSTRM_VIDEO_STATS_VERSION 1


/* Video statistics - static */
struct vstrm_video_stats {
	/* Video statistics version, see VSTRM_VIDEO_STATS_VERSION */
	uint8_t version;

	/* Wifi RSSI (dBm) (0 if unknown) */
	int8_t rssi;

	/* Timestamp associated with the video statistics (us, monotonic) */
	uint64_t timestamp;

	/* Total frame counter including all output, discarded and
	 * missing frames */
	uint32_t total_frame_count;

	/* Output frame counter; output frames are frames that are output
	 * from the library, regardless of their valid or errored status */
	uint32_t output_frame_count;

	/* Errored output frame counter (included in output_frame_count);
	 * errored output frames are frames output from the library that
	 * contain errors (either incomplete frames with or without error
	 * concealment, or frames within error propagation) */
	uint32_t errored_output_frame_count;

	/* Missed frame counter; missed frames are frames that either have
	 * been discarded or that are completely absent from the received
	 * stream (note: absent non-reference frames are not counted as missed
	 * frames, as there is no error propagation) */
	uint32_t missed_frame_count;

	/* Discarded frame counter (included in missed_frame_count); discarded
	 * frames are frames that are not output because they are incomplete
	 * and error concealment failed */
	uint32_t discarded_frame_count;

	/* Frame timestamp delta integral value; timestamp delta is the time
	 * difference between two consecutive output frames acquisition
	 * timestamps */
	uint64_t timestamp_delta_integral;

	/* Frame timestamp delta squared integral value */
	uint64_t timestamp_delta_integral_sq;

	/* Frame timing error integral value; the timing error is the absolute
	 * difference between acquisition timestamp delta and output timestamp
	 * delta for two consecutive output frames */
	uint64_t timing_error_integral;

	/* Frame timing error squared integral value */
	uint64_t timing_error_integral_sq;

	/* Frame estimated latency integral value; the estimated latency is
	 * the difference between a frame output timestamp and acquisition
	 * timestamp using an estimation of the clock difference between the
	 * sender and the receiver */
	uint64_t estimated_latency_integral;

	/* Frame estimated latency squared integral value */
	uint64_t estimated_latency_integral_sq;

	/* Errored second counter (on whole frames) */
	uint32_t errored_second_count;

	/* Macroblock status class count */
	uint32_t mb_status_class_count;

	/* Macroblock status zone count */
	uint32_t mb_status_zone_count;
};


/* Video statistics - dynamic */
struct vstrm_video_stats_dyn {
	/* Macroblock status class count */
	uint32_t mb_status_class_count;

	/* Macroblock status zone count */
	uint32_t mb_status_zone_count;

	/* Errored second count by zone; the array size is
	 * mb_status_zone_count */
	uint32_t *errored_second_count_by_zone;

	/* Macroblock status population count by zone; the array dimensions
	 * are mb_status_class_count * mb_status_zone_count */
	uint32_t *macroblock_status;
};


/**
 * Initialize a dynamic video statistics structure.
 * When no longer needed the structure must be freed using the
 * vstrm_video_stats_dyn_clear() function.
 * @param dyn: pointer to a dynamic video statistics structure (output)
 * @param mbStatusClassCount: macroblock status class count
 * @param mbStatusZoneCount: macroblock status zone count
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_video_stats_dyn_init(struct vstrm_video_stats_dyn *dyn,
			       uint32_t mb_status_class_count,
			       uint32_t mb_status_zone_count);


/**
 * Clear a dynamic video statistics structure.
 * This function frees the resources used by the video statistics structure.
 * The structure itself is not freed by the function, its ownership stays
 * with the caller.
 * @param dyn: pointer to a dynamic video statistics structure
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_video_stats_dyn_clear(struct vstrm_video_stats_dyn *dyn);


/**
 * Copy a dynamic video statistics structure.
 * This function initializes the dst video statistics structure with the
 * same contents as the src structure (deep copy).
 * When no longer needed the dst structure must be freed using the
 * vstrm_video_stats_dyn_clear() function.
 * @param dst: pointer to the destination dynamic video statistics
 *             structure (output)
 * @param src: pointer to the source dynamic video statistics structure
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_video_stats_dyn_copy(struct vstrm_video_stats_dyn *dst,
			       const struct vstrm_video_stats_dyn *src);


#endif /* !_VSTRM_VIDEO_STATS_H_ */
