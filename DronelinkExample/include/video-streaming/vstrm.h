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

#ifndef _VSTRM_H_
#define _VSTRM_H_

#include <stdint.h>

#include <futils/list.h>
#include <libpomp.h>
#include <video-metadata/vmeta.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** To be used for all public API */
#ifdef VSTRM_API_EXPORTS
#	ifdef _WIN32
#		define VSTRM_API __declspec(dllexport)
#	else /* !_WIN32 */
#		define VSTRM_API __attribute__((visibility("default")))
#	endif /* !_WIN32 */
#else /* !VSTRM_API_EXPORTS */
#	define VSTRM_API
#endif /* !VSTRM_API_EXPORTS */


/* Forward declarations */
struct rtp_pkt;
struct rtcp_pkt_receiver_report;


#include "video-streaming/vstrm_h264_sei_streaming.h"
#include "video-streaming/vstrm_video_stats.h"

#include "video-streaming/vstrm_frame.h"
#include "video-streaming/vstrm_receiver.h"
#include "video-streaming/vstrm_sender.h"


/**
 * Debug files flags; can be set in configuration as well as in
 * VSTRM_DBG_FLAGS environment variables (with VSTRM_DBG_DIR specifying the
 * output directory
 */

/* RTP packets received */
#define VSTRM_DBG_FLAG_RECEIVER_RTP_IN (1 << 0)

/* RTP packets received after jitter buffer */
#define VSTRM_DBG_FLAG_RECEIVER_RTP_JITTER (1 << 1)

/* Video stream for the decoder */
#define VSTRM_DBG_FLAG_RECEIVER_STREAM (1 << 2)

/* Video stream from the encoder */
#define VSTRM_DBG_FLAG_SENDER_STREAM (1 << 8)

/* RTP packets constructed */
#define VSTRM_DBG_FLAG_SENDER_RTP_PAYLOAD (1 << 9)

/* RTP packets really sent (after potential drops and reordering) */
#define VSTRM_DBG_FLAG_SENDER_RTP_OUT (1 << 10)

/* Clock delta algorithm CSV output */
#define VSTRM_DBG_FLAG_CLOCK_DELTA (1 << 16)

/* Video stats CSV output */
#define VSTRM_DBG_FLAG_VIDEO_STATS (1 << 17)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _VSTRM_H_ */
