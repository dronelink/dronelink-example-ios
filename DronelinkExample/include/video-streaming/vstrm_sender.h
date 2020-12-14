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

#ifndef _VSTRM_SENDER_H_
#define _VSTRM_SENDER_H_


/* Forward declaration */
struct vstrm_sender;


/**
 * Sender configuration flags
 */

/* Enable the serializing of frame metadata as RTP header extensions */
#define VSTRM_SENDER_FLAGS_ENABLE_RTP_HEADER_EXT (1 << 0)

/* Enable RTCP */
#define VSTRM_SENDER_FLAGS_ENABLE_RTCP (1 << 1)

/* Enable RTCP extensions (Parrot clock delta algorithm) */
#define VSTRM_SENDER_FLAGS_ENABLE_RTCP_EXT (1 << 2)

/* Enable raw sender */
#define VSTRM_SENDER_FLAGS_RAW (1 << 3)


/* Sender dynamic configuration */
struct vstrm_sender_cfg_dyn {
	/* Target network packet size in bytes */
	uint32_t target_packet_size;

	/* Maximum acceptable total latency in ms for each importance level
	 * (no drop if 0); the total latency is the difference between the
	 * frame capture TS and the output time on the network */
	uint32_t max_total_latency_ms[VSTRM_FRAME_MAX_NALU_IMPORTANCE_LEVELS];

	/* Maximum acceptable network latency in ms for each importance level
	 * (no drop if 0); the network latency is the difference between the
	 * frame input TS in the sender and the output time on the network */
	uint32_t max_network_latency_ms[VSTRM_FRAME_MAX_NALU_IMPORTANCE_LEVELS];
};


/* Sender initial configuration */
struct vstrm_sender_cfg {
	/* Event loop to use */
	struct pomp_loop *loop;

	/* Configuration flags, see VSTRM_SENDER_FLAGS_xxx */
	uint32_t flags;

	/* Self session metadata: serial_number is mandatory, friendly_name and
	 * software_version are recommended, other fields are optional */
	struct vmeta_session self_meta;

	/* Dynamic configuration */
	struct vstrm_sender_cfg_dyn dyn;

	/* Debug output directory */
	const char *dbg_dir;

	/* Debug flags */
	uint32_t dbg_flags;
};


/* Sender callback functions */
struct vstrm_sender_cbs {
	/* Called when a data (RTP) packet needs to be sent.
	 * The sender has a reference on the buffer and it must not be
	 * unreferenced by the callback function.
	 * @param stream: sender instance pointer
	 * @param buf: pointer to the buffer to send
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*send_data)(struct vstrm_sender *stream,
			 struct pomp_buffer *buf,
			 void *userdata);

	/* Called when a control (RTCP) packet needs to be sent.
	 * The sender has a reference on the buffer and it does not need to be
	 * unreferenced by the callback function.
	 * @param stream: sender instance pointer
	 * @param buf: pointer to the buffer to send
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*send_ctrl)(struct vstrm_sender *stream,
			 struct pomp_buffer *buf,
			 void *userdata);

	/* Called to enable/disable the output availability monitoring (ready
	 * to send) on the data (RTP) channel.
	 * @param stream: sender instance pointer
	 * @param enable: 1: enable output availability monitoring, 0: disable
	 * @param userdata: user data pointer
	 * @return 0 on success, negative errno value in case of error */
	int (*monitor_send_data_ready)(struct vstrm_sender *stream,
				       int enable,
				       void *userdata);

	/* Called when the peer session metadata has changed.
	 * @param stream: sender instance pointer
	 * @param meta: pointer to the updated session metadata structure
	 * @param userdata: user data pointer */
	void (*session_metadata_peer_changed)(struct vstrm_sender *stream,
					      const struct vmeta_session *meta,
					      void *userdata);

	/* Called when a RTCP receiver report been received from the receiver.
	 * @param stream: sender instance pointer
	 * @param rr: pointer to the received receiver report structure
	 * @param userdata: user data pointer */
	void (*receiver_report)(struct vstrm_sender *stream,
				const struct rtcp_pkt_receiver_report *rr,
				void *userdata);

	/* Called when video statistics have been received from the receiver.
	 * @param stream: sender instance pointer
	 * @param video_stats: pointer to the received video statistics
	 *                     structure (static)
	 * @param video_stats_dyn: pointer to the received video statistics
	 *                         structure (dynamic)
	 * @param userdata: user data pointer */
	void (*video_stats)(struct vstrm_sender *stream,
			    const struct vstrm_video_stats *video_stats,
			    const struct vstrm_video_stats_dyn *video_stats_dyn,
			    void *userdata);

	/* Called when a RTCP goodbye packet has been received from
	 * the receiver.
	 * @param stream: sender instance pointer
	 * @param reason: optional pointer to the RTCP BYE reason string
	 *                (can be NULL)
	 * @param userdata: user data pointer */
	void (*goodbye)(struct vstrm_sender *stream,
			const char *reason,
			void *userdata);
};


/* Sender statistics */
struct vstrm_sender_stats {
	/* Overall total packet count */
	uint32_t total_packet_count;

	/* Overall total byte count */
	uint32_t total_byte_count;

	/* Overall dropped packet count */
	uint32_t dropped_packet_count;

	/* Overall dropped byte count */
	uint32_t dropped_byte_count;
};


/**
 * Create a sender instance.
 * The configuration and callbacks structures must be filled.
 * The instance handle is returned through the ret_obj parameter.
 * When no longer needed, the instance must be freed using the
 * vstrm_sender_destroy() function.
 * @param cfg: sender configuration
 * @param cbs: sender callback functions
 * @param userdata: callback functions user data (optional, can be NULL)
 * @param ret_obj: sender instance handle (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_new(const struct vstrm_sender_cfg *cfg,
		     const struct vstrm_sender_cbs *cbs,
		     void *userdata,
		     struct vstrm_sender **ret_obj);


/**
 * Free a sender instance.
 * This function frees all resources associated with a sender instance.
 * @param self: sender instance handle
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_destroy(struct vstrm_sender *self);


/**
 * Send a video frame.
 * The payloader serializes the frame data as RTP and packets are sent
 * using the send_data() callback function. The frame must have been
 * previously allocated using the vstrm_frame_new() function. It is the
 * caller's responsibility to unreference the frame after returning from
 * this function once it is no longer needed.
 * When in raw sender mode, this function returns a -EPERM error.
 * @param self: sender instance handle
 * @param frame: pointer to the frame to send
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_send_frame(struct vstrm_sender *self,
			    struct vstrm_frame *frame);


/**
 * Send a data (RTP) packet.
 * The packet must have been previously allocated using the rtp_pkt_new()
 * function. The ownership of the buffer is transfered to the library and the
 * buffer must not be freed by the caller after returning from this function.
 * When not in raw sender mode, this function returns a -EPERM error.
 * @param self: sender instance handle
 * @param pkt: pointer to the packet to send
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_send_rtp_pkt(struct vstrm_sender *self, struct rtp_pkt *pkt);


/**
 * Send a RTCP goodbye packet.
 * An optional reason string can be supplied. The reason string ownership stays
 * with the caller, it is copied internally if necessary.
 * @param self: sender instance handle
 * @param reason: optional pointer to a RTCP BYE reason string (can be NULL)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_send_goodbye(struct vstrm_sender *self, const char *reason);


/**
 * Receive a control (RTCP) packet.
 * The ownership of the buffer stays with the caller.
 * @param self: sender instance handle
 * @param buf: pointer to the received buffer
 * @param ts: packet reception timestamp
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_recv_ctrl(struct vstrm_sender *self,
			   struct pomp_buffer *buf,
			   const struct timespec *ts);


/**
 * Notify that the data channel (RTP) is ready for sending.
 * @param self: sender instance handle
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_notify_send_data_ready(struct vstrm_sender *self);


/**
 * Get the dynamic sender configuration.
 * The caller must provide a configuration structure to fill.
 * @param self: sender instance handle
 * @param cfg_dyn: pointer to a dynamic configuration structure (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_get_cfg_dyn(struct vstrm_sender *self,
			     struct vstrm_sender_cfg_dyn *cfg_dyn);


/**
 * Set the dynamic sender configuration.
 * The caller must provide a filled configuration structure.
 * @param self: sender instance handle
 * @param cfg_dyn: pointer to the new dynamic configuration structure
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_set_cfg_dyn(struct vstrm_sender *self,
			     const struct vstrm_sender_cfg_dyn *cfg_dyn);


/**
 * Get RTP parameters for the next frame to send.
 * @param self: sender instance handle
 * @param timestamp: frame timestamp (in us)
 * @param seq: pointer to the value of the first sequence number of the
 *             first packet of the next frame (output)
 * @param rtpts: pointer to the value of the RTP timestamp of the next
 *               frame (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_get_next_frame_params(struct vstrm_sender *self,
				       uint64_t timestamp,
				       uint16_t *seq,
				       uint32_t *rtpts);


/**
 * Get the sender RTP synchronization source identifier (SSRC).
 * @param self: sender instance handle
 * @param ssrc: pointer to the value of the sender SSRC (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_get_ssrc_self(struct vstrm_sender *self, uint32_t *ssrc);


/**
 * Get the receiver (peer) RTP synchronization source identifier (SSRC).
 * @param self: sender instance handle
 * @param ssrc: pointer to the value of the receiver SSRC (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_get_ssrc_peer(struct vstrm_sender *self, uint32_t *ssrc);


/**
 * Get the sender statistics.
 * The caller must provide a statistics structure to fill.
 * @param self: sender instance handle
 * @param stats: pointer to a sender statistics structure (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_get_stats(struct vstrm_sender *self,
			   struct vstrm_sender_stats *stats);


/**
 * Set the sender session metadata.
 * The caller must provide a filled session metadata structure. The metadata
 * is copied internally and the meta structure can be freed when returning
 * from the function.
 * @param self: sender instance handle
 * @param meta: pointer to the new session metadata structure
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_set_session_metadata_self(struct vstrm_sender *self,
					   const struct vmeta_session *meta);


/**
 * Get the sender session metadata.
 * The caller must provide a session metadata structure to fill.
 * @param self: sender instance handle
 * @param meta: pointer to a session metadata structure (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_get_session_metadata_self(struct vstrm_sender *self,
					   const struct vmeta_session **meta);


/**
 * Get the receiver (peer) session metadata.
 * The caller must provide a session metadata structure to fill.
 * @param self: sender instance handle
 * @param meta: pointer to a session metadata structure (output)
 * @return 0 on success, negative errno value in case of error
 */
VSTRM_API
int vstrm_sender_get_session_metadata_peer(struct vstrm_sender *self,
					   const struct vmeta_session **meta);


#endif /* !_VSTRM_SENDER_H_ */
