/*
 * encoder.h
 *
 *  Created on: Oct 20, 2022
 *      Author: bhat
 */

#ifndef SRC_VIRTIO_ENCODER_H_
#define SRC_VIRTIO_ENCODER_H_

#include "../event.h"

#include "xil_types.h"

struct VirtEncoder;
void virt_encoder_initialize(
	struct VirtEncoder* self,
	struct EventLoop* event_loop
);
void virt_encoder_handle_event(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
);
void virt_encoder_tick(
	struct VirtEncoder* self,
	u32 ts_now
);

enum VirtEncoderState {
	virt_encoder_Idle,

	virt_encoder_B0, virt_encoder_B1,
	virt_encoder_C0, virt_encoder_C1,
	virt_encoder_D0, virt_encoder_D1,

	virt_encoder_Unknown
};

struct VirtEncoder {
	struct EventLoop* event_loop;

	u32 latency;

	enum VirtEncoderState state;
	u32 state_ts;

	// Signals
	struct Signal on_clockwise;
	struct Signal on_counterclockwise;
	struct Signal on_unknown;
	struct Signal on_state_cleared;
};

struct VirtEncoder virt_encoder_new(
	u32 latency
);

#endif /* SRC_VIRTIO_ENCODER_H_ */
