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

#ifndef _VSTRM_RECEIVER_H_
#define _VSTRM_RECEIVER_H_


/* Forward declaration */
struct vstrm_receiver;


/**
 * Receiver configuration flags
 */

/* Enable RTCP */
#define VSTRM_RECEIVER_FLAGS_ENABLE_RTCP (1 << 0)

/* Enable RTCP extensions (Parrot video statistics and
 * Parrot clock delta algorithm) */
#define VSTRM_RECEIVER_FLAGS_ENABLE_RTCP_EXT (1 << 1)

/* Generate H.264 skipped P slices for error concealment on missing slices */
#define VSTRM_RECEIVER_FLAGS_H264_GEN_CONCEALMENT_SLICE (1 << 4)

/* Generate a H.264 grey IDR frame at the stream beginning to initialize
 * the decoding (for intra-refresh streams) */
#define VSTRM_RECEIVER_FLAGS_H264_GEN_GREY_IDR_FRAME (1 << 5)

/* Fake H.264 frame_num syntax elements in slice headers to force contiguous
 * frame numbers at the decoder input */
#define VSTRM_RECEIVER_FLAGS_H264_FAKE_FRAME_NUM (1 << 6)

/* Compute full macroblock status by parsing all macroblocks instead of just
 * looking at the slice type (to detect refresh patterns in P-slices).
 * This can generate high CPU load so it should be used mainly for tests.
 * Can also be activated via environment variable (with same name) */
#define VSTRM_RECEIVER_FLAGS_H264_FULL_MB_STATUS (1 << 31)


/* Receiver configuration */
struct vstrm_receiver_cfg {
	/* Event loop to use */
	struct pomp_loop *loop;

	/* Configuration flags, see VSTRM_RECEIVER_FLAGS_xxx */
	uint32_t flags;

	/* Self session metadata: serial_number is mandatory, friendly_name
	 * and software_version are recommended, maker, model and build_id are
	 * optional, other fields are not used on the receiver */
	struct vmeta_session self_meta;

	/* Debug output directory */
	const char *dbg_dir;

	/* Debug flags */
	uint32_t dbg_flags;
};


/* Receiver callback functions */
struct vstrm_receiver_cbs {
	/* Called when a control (RCTP) packet needs to be sent.
	 * The receiver has a reference on the buffer and it must not be
	 * unreferenced by the callback function.
	 * @param stream: receiver instance pointer
	 * @param buf: pointer to the buffer to send
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*send_ctrl)(struct vstrm_receiver *stream,
			 struct pomp_buffer *buf,
			 void *userdata);

	/* Called when the codec information has changed.
	 * @param stream: receiver instance pointer
	 * @param info: pointer to the updated codec information structure
	 * @param userdata: user data pointer */
	void (*codec_info_changed)(struct vstrm_receiver *stream,
				   const struct vstrm_codec_info *info,
				   void *userdata);

	/* Called when a frame has been received.
	 * The ownership of the frame stays within the library. If the
	 * application requires to use the frame after returning from the
	 * callback function, it must reference it in the callback function
	 * and later unreference it when no longer needed.
	 * @param stream: receiver instance pointer
	 * @param frame: pointer to the received video frame structure
	 * @param userdata: user data pointer */
	void (*recv_frame)(struct vstrm_receiver *stream,
			   struct vstrm_frame *frame,
			   void *userdata);

	/* Called when a RTP packet has been received (before depayloading
	 * the media).
	 * The ownership of the packet stays within the library. If the
	 * application requires to use the packet after returning from the
	 * callback function, it must reference it in the callback function
	 * and later unreference it when no longer needed.
	 * @param stream: receiver instance pointer
	 * @param pkt: pointer to the received packet structure
	 * @param userdata: user data pointer */
	void (*recv_rtp_pkt)(struct vstrm_receiver *stream,
			     const struct rtp_pkt *pkt,
			     void *userdata);

	/* Called when the peer session metadata has changed.
	 * @param stream: receiver instance pointer
	 * @param meta: pointer to the updated session metadata structure
	 * @param userdata: user data pointer */
	void (*session_metadata_peer_changed)(struct vstrm_receiver *stream,
					      const struct vmeta_session *meta,
					      void *userdata);

	/* Called when a RTCP goodbye packet has been received from
	 * the sender.
	 * @param stream: receiver instance pointer
	 * @param reason: optional pointer to the RTCP BYE reason string
	 *                (can be NULL)
	 * @param userdata: user data pointer */
	void (*goodbye)(struct vstrm_receiver *stream,
			const char *reason,
			void *userdata);
};


/* Receiver statistics */
struct vstrm_receiver_stats {
	/* Overall received packet count */
	uint32_t received_packet_count;

	/* Overall received byte count */
	uint32_t received_byte_count;

	/* Overall lost packet count */
	uint32_t lost_packet_count;
};


/**
 * Create a receiver instance.
 * The configuration and callbacks structures must be filled.
 * The instance handle is returned through the ret_obj parameter.
 * When no longer needed, the instance must be freed using the
 * vstrm_receiver_destroy() function.
 * @param cfg: receiver configuration
 * @param cbs: receiver callback functions
 * @param userdata: callback functions user data (optional, can be NULL)
 * @param ret_obj: receiver instance handle (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_new(const struct vstrm_receiver_cfg *cfg,
		       const struct vstrm_receiver_cbs *cbs,
		       void *userdata,
		       struct vstrm_receiver **ret_obj);


/**
 * Free a receiver instance.
 * This function frees all resources associated with a receiver instance.
 * @param self: receiver instance handle
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_destroy(struct vstrm_receiver *self);


/**
 * Receive a data (RTP) packet.
 * The ownership of the buffer stays within the caller.
 * @param self: receiver instance handle
 * @param buf: pointer to the received buffer
 * @param ts: packet reception timestamp
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_recv_data(struct vstrm_receiver *self,
			     struct pomp_buffer *buf,
			     const struct timespec *ts);


/**
 * Receive a control (RTCP) packet.
 * The ownership of the buffer stays within the caller.
 * @param self: receiver instance handle
 * @param buf: pointer to the received buffer
 * @param ts: packet reception timestamp
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_recv_ctrl(struct vstrm_receiver *self,
			     struct pomp_buffer *buf,
			     const struct timespec *ts);


/**
 * Send a RTCP goodbye packet.
 * An optional reason string can be supplied. The reason string ownership stays
 * within the caller, it is copied internally if necessary.
 * @param self: receiver instance handle
 * @param reason: optional pointer to a RTCP BYE reason string (can be NULL)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_send_goodbye(struct vstrm_receiver *self,
				const char *reason);


/**
 * Set the codec information.
 * Used when codec information is available out of band (eg. through SDP).
 * @param self: receiver instance handle
 * @param info: pointer to a codec information structure
 * @param ssrc: sender SSRC
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_set_codec_info(struct vstrm_receiver *self,
				  const struct vstrm_codec_info *info,
				  uint32_t ssrc);


/**
 * Generate a H.264 grey IDR frame.
 * Used to generate a H.264 grey IDR frame to synchronize a video decoder
 * (for intra-refresh streams).
 * @param self: receiver instance handle
 * @param ret_frame: pointer to a frame (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_generate_grey_i_frame(struct vstrm_receiver *self,
					 struct vstrm_frame **ret_frame);


/**
 * Get the NTP timestamp from a RTP timestamp.
 * @param self: receiver instance handle
 * @param rtpts: RTP timestamp
 * @return the NTP timestamp on success, 0 in case of error
 */
VSTRM_API
uint64_t vstrm_receiver_get_ntp_from_rtp_ts(struct vstrm_receiver *self,
					    uint32_t rtpts);


/**
 * Get the receiver RTP synchronization source identifier (SSRC).
 * @param self: receiver instance handle
 * @param ssrc: pointer to the value of the receiver SSRC (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_get_ssrc_self(struct vstrm_receiver *self, uint32_t *ssrc);


/**
 * Get the sender (peer) RTP synchronization source identifier (SSRC).
 * @param self: receiver instance handle
 * @param ssrc: pointer to the value of the sender SSRC (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_get_ssrc_peer(struct vstrm_receiver *self, uint32_t *ssrc);


/**
 * Get the receiver statistics.
 * The caller must provide a statistics structure to fill.
 * @param self: receiver instance handle
 * @param stats: pointer to a receiver statistics structure (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_get_stats(struct vstrm_receiver *self,
			     struct vstrm_receiver_stats *stats);


/**
 * Set the receiver session metadata.
 * The caller must provide a filled session metadata structure. The metadata
 * is copied internally and the meta structure can be freed when returning
 * from the function.
 * @param self: receiver instance handle
 * @param meta: pointer to the new session metadata structure
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_set_session_metadata_self(struct vstrm_receiver *self,
					     const struct vmeta_session *meta);


/**
 * Get the receiver session metadata.
 * The caller must provide a session metadata structure to fill.
 * @param self: receiver instance handle
 * @param meta: pointer to a session metadata structure (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_get_session_metadata_self(struct vstrm_receiver *self,
					     const struct vmeta_session **meta);


/**
 * Get the sender (peer) session metadata.
 * The caller must provide a session metadata structure to fill.
 * @param self: receiver instance handle
 * @param meta: pointer to a session metadata structure (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_receiver_get_session_metadata_peer(struct vstrm_receiver *self,
					     const struct vmeta_session **meta);


#endif /* !_VSTRM_RECEIVER_H_ */
