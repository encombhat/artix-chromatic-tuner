/*
 * encoder.c
 *
 *  Created on: Oct 20, 2022
 *      Author: bhat
 */

#include "encoder.h"

struct VirtEncoder virt_encoder_new(u32 latency) {
	struct VirtEncoder encoder = {
		.event_loop = 0,

		.latency = latency,

		.state = virt_encoder_Idle,
		.state_ts = 0,

		.on_clockwise = signal_new(),
		.on_counterclockwise = signal_new(),
		.on_unknown = signal_new(),
		.on_state_cleared = signal_new(),
	};

	return encoder;
}

void virt_encoder_initialize(
	struct VirtEncoder* self,
	struct EventLoop* event_loop
) {
	self->event_loop = event_loop;
}

void _virt_encoder_update_state(
	struct VirtEncoder* self,
	enum VirtEncoderState new_state
) {
	if (new_state == virt_encoder_Unknown) {
		signal_trigger(
			&(self->on_unknown),
			self->event_loop, 0
		);
	}

	self->state = new_state;
}

#define _VIRT_ENCODER_NN 0
#define _VIRT_ENCODER_NP 1
#define _VIRT_ENCODER_PN 2
#define _VIRT_ENCODER_PP 3

// PP
void _virt_encoder_handle_idle(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
) {
	switch (event_data) {
	case _VIRT_ENCODER_PN:
//		xil_printf("a->b0\r\n");
		_virt_encoder_update_state(self, virt_encoder_B0);
		break;
	case _VIRT_ENCODER_NP:
//		xil_printf("a->b1\r\n");
		_virt_encoder_update_state(self, virt_encoder_B1);
		break;
	case _VIRT_ENCODER_PP:
		break; // Why is this even here
	case _VIRT_ENCODER_NN: // too fast!
	default:
		_virt_encoder_update_state(self, virt_encoder_Unknown);
	}
}

// PN
void _virt_encoder_handle_b0(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
) {
	switch (event_data) {
	case _VIRT_ENCODER_NN:
//		xil_printf("b0->c0\r\n");
		_virt_encoder_update_state(self, virt_encoder_C0);
		break;
	case _VIRT_ENCODER_PP:
//		xil_printf("b0->a\r\n");
		_virt_encoder_update_state(self, virt_encoder_Idle);
		break;
	case _VIRT_ENCODER_PN:
		break;
	default:
		_virt_encoder_update_state(self, virt_encoder_Unknown);
	}
}

// NP
void _virt_encoder_handle_b1(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
) {
	switch (event_data) {
	case _VIRT_ENCODER_NN:
//		xil_printf("b1->c1\r\n");
		_virt_encoder_update_state(self, virt_encoder_C1);
		break;
	case _VIRT_ENCODER_PP:
//		xil_printf("b1->a\r\n");
		_virt_encoder_update_state(self, virt_encoder_Idle);
		break;
	case _VIRT_ENCODER_NP:
		break;
	default:
		_virt_encoder_update_state(self, virt_encoder_Unknown);
	}
}

// NN
void _virt_encoder_handle_c0(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
) {
	switch (event_data) {
	case _VIRT_ENCODER_NP:
//		xil_printf("c0->d0\r\n");
		_virt_encoder_update_state(self, virt_encoder_D0);
		break;
	case _VIRT_ENCODER_PN: // Assuming bounce
		break;
	case _VIRT_ENCODER_PP:
//		xil_printf("c0->a\r\n");
		_virt_encoder_update_state(self, virt_encoder_Idle);
		signal_trigger(
			&(self->on_counterclockwise),
			self->event_loop, 0
		);
		break;
	case _VIRT_ENCODER_NN:
		break;
	default:
		_virt_encoder_update_state(self, virt_encoder_Unknown);
	}
}

// NN
void _virt_encoder_handle_c1(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
) {
	switch (event_data) {
	case _VIRT_ENCODER_PN:
//		xil_printf("c1->d1\r\n");
		_virt_encoder_update_state(self, virt_encoder_D1);
		break;
	case _VIRT_ENCODER_NP: // Assuming bounce
		break;
	case _VIRT_ENCODER_PP:
//		xil_printf("c1->a\r\n");
		_virt_encoder_update_state(self, virt_encoder_Idle);
		signal_trigger(
			&(self->on_clockwise),
			self->event_loop, 0
		);
		break;
	case _VIRT_ENCODER_NN:
		break;
	default:
		_virt_encoder_update_state(self, virt_encoder_Unknown);
	}
}

// NP
void _virt_encoder_handle_d0(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
) {
	switch (event_data) {
	case _VIRT_ENCODER_PP:
//		xil_printf("d0->a\r\n");
		_virt_encoder_update_state(self, virt_encoder_Idle);
		signal_trigger(
			&(self->on_counterclockwise),
			self->event_loop, 0
		);
		break;
	case _VIRT_ENCODER_NN:
//		_virt_encoder_update_state(self, virt_encoder_C0);
		break;
	case _VIRT_ENCODER_NP:
		break;
	default:
		_virt_encoder_update_state(self, virt_encoder_Unknown);
	}
}

// PN
void _virt_encoder_handle_d1(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
) {
	switch (event_data) {
	case _VIRT_ENCODER_PP:
//		xil_printf("d1->a\r\n");
		_virt_encoder_update_state(self, virt_encoder_Idle);
		signal_trigger(
			&(self->on_clockwise),
			self->event_loop, 0
		);
		break;
	case _VIRT_ENCODER_NN:
//		_virt_encoder_update_state(self, virt_encoder_C1);
		break;
	case _VIRT_ENCODER_PN:
		break;
	default:
		_virt_encoder_update_state(self, virt_encoder_Unknown);
	}
}

void _virt_encoder_handle_default(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
) {
	switch (event_data) {
	case _VIRT_ENCODER_PP:
		_virt_encoder_update_state(self, virt_encoder_Idle);
		break;
	default:
		break;
	}
}

void virt_encoder_handle_event(
	struct VirtEncoder* self,
	u32 event_data, u32 event_ts
) {
	self->state_ts = event_ts;

	switch (self->state) {
	case virt_encoder_Idle:
		_virt_encoder_handle_idle(
			self, event_data, event_ts
		);
		break;
	case virt_encoder_B0:
		_virt_encoder_handle_b0(
			self, event_data, event_ts
		);
		break;
	case virt_encoder_B1:
		_virt_encoder_handle_b1(
			self, event_data, event_ts
		);
		break;
	case virt_encoder_C0:
		_virt_encoder_handle_c0(
			self, event_data, event_ts
		);
		break;
	case virt_encoder_C1:
		_virt_encoder_handle_c1(
			self, event_data, event_ts
		);
		break;
	case virt_encoder_D0:
		_virt_encoder_handle_d0(
			self, event_data, event_ts
		);
		break;
	case virt_encoder_D1:
		_virt_encoder_handle_d1(
			self, event_data, event_ts
		);
		break;
	default:
		_virt_encoder_handle_default(
			self, event_data, event_ts
		);
		break;
	}
}

void virt_encoder_tick(
	struct VirtEncoder* self,
	u32 ts_now
) {
	if (self->state == virt_encoder_Idle) return;
	if (ts_now - self->state_ts < self->latency) return;

//	xil_printf("Clearing virt_encoder state...\r\n");

	_virt_encoder_update_state(self, virt_encoder_Idle);
	self->state_ts = ts_now;

	signal_trigger(
		&(self->on_state_cleared),
		self->event_loop, 0
	);
}
