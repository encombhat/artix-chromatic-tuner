/*
 * encoder.c
 *
 *  Created on: Oct 19, 2022
 *      Author: bhat
 */

#include "encoder.h"

void _encoder_handler(struct Encoder* self);

struct Encoder encoder_new(
	u8 device_id, u8 intr_id,
	u32* ts_now_ptr
) {
	struct Encoder encoder = {
		.event_loop = 0,

		.intrdev = {
			.device_id = device_id,
			.intr_id = intr_id,
		},
		.ts_now_ptr = ts_now_ptr,

		.buf = {0},
		.buf_aux = {
			.size = ENCODER_EVENT_BUF_SIZE,
			.head = 0, .tail = 0,
		},
		.last_event = 0,

		.intrcon = intrcon_new(),
		.sys_gpio = {},

		.on_triggered = signal_new(),
	};

	return encoder;
}

void encoder_initialize(
	struct Encoder* self,
	struct EventLoop* event_loop
) {
	intrcon_initialize(&(self->intrcon));

	XGpio_Initialize(&(self->sys_gpio), self->intrdev.device_id);
	XGpio_InterruptEnable(&(self->sys_gpio), 1);
	XGpio_InterruptGlobalEnable(&(self->sys_gpio));

	self->event_loop = event_loop;
}

void encoder_register(struct Encoder* self) {
	intrcon_register_device(
		&(self->intrcon),
		&(self->intrdev),
		(XInterruptHandler)_encoder_handler,
		(void*)self
	);
	intrcon_start_controller(&(self->intrcon));
}

#define _ENCODER_HANDLER_MAX_ITERATION 256
#define _ENCODER_HANDLER_MAX_DATA ENCODER_EVENT_BUF_SIZE / 2

// A slow interrupt handler, does not matter.
void _encoder_handler(struct Encoder* self) {
	u32 event_count = 0;

	for (u32 i = 0; i < _ENCODER_HANDLER_MAX_ITERATION; i += 1) {
		if (event_count == _ENCODER_HANDLER_MAX_DATA) break;
		u32 event_data = XGpio_DiscreteRead(&(self->sys_gpio), 1);
		if (event_data == self->last_event) continue;

		event_count += 1;
		self->last_event = event_data;

		// Since auxiliary data can only store 4 bytes,
		// compress encoder signal and time stamp into one u32
		u32 aux_data = *(self->ts_now_ptr);
		aux_data <<= 4; // Shift time stamp left by 4
		aux_data |= (event_data & 0xF); // event data is 4 bits

		buf_push((u32*) self->buf, aux_data, self->buf_aux);
		bufaux_push(&(self->buf_aux));
	}

	XGpio_InterruptClear(&(self->sys_gpio), 1);
}

bool encoder_pressed_rt(struct Encoder* self) {
	u32 event_data = XGpio_DiscreteRead(&(self->sys_gpio), 1);
	return (event_data & 0x4) > 0;
}

void encoder_idle(struct Encoder* self) {
	while (!buf_empty(self->buf_aux)) {
		u32 aux_data = buf_pop(self->buf, self->buf_aux);
		bufaux_pop(&(self->buf_aux));

		signal_trigger(
			&(self->on_triggered), self->event_loop,
			(void*) aux_data
		);
	}
}
