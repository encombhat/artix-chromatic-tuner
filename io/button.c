/*
 * button.c
 *
 *  Created on: Oct 29, 2022
 *      Author: bhat
 */

#include "button.h"

struct Button button_new(
	u8 device_id,
	u8 intr_id
) {
	struct Button btn = {
		.event_loop = 0,

		.intrdev = {
			.device_id = device_id,
			.intr_id = intr_id,
		},

		.buf = {0},
		.buf_aux = {
			.size = BUTTON_EVENT_BUF_SIZE,
			.head = 0, .tail = 0,
		},

		.intrcon = intrcon_new(),
		.sys_gpio = {},

		.on_triggered = signal_new(),
	};

	return btn;
}

void _button_handler(struct Button* self);

void button_initialize(
	struct Button* self,
	struct EventLoop* event_loop
) {
	intrcon_initialize(&(self->intrcon));

	XGpio_Initialize(&(self->sys_gpio), self->intrdev.device_id);
	XGpio_InterruptEnable(&(self->sys_gpio), 1);
	XGpio_InterruptGlobalEnable(&(self->sys_gpio));

	self->event_loop = event_loop;
}

void button_register(struct Button* self) {
	intrcon_register_device(
		&(self->intrcon),
		&(self->intrdev),
		(XInterruptHandler)_button_handler,
		(void*)self
	);
	intrcon_start_controller(&(self->intrcon));
}

void _button_handler(struct Button* self) {
	u32 data = XGpio_DiscreteRead(&(self->sys_gpio), 1);
//	xil_printf("button touched: %d\r\n", data);

	buf_push((u32*) self->buf, data, self->buf_aux);
	bufaux_push(&(self->buf_aux));

	XGpio_InterruptClear(&(self->sys_gpio), 1);
}

u32 button_event_rt(struct Button* self) {
	return XGpio_DiscreteRead(&(self->sys_gpio), 1);
}

void button_idle(struct Button* self) {
	while (!buf_empty(self->buf_aux)) {
		u32 aux_data = buf_pop(self->buf, self->buf_aux);
		bufaux_pop(&(self->buf_aux));

		signal_trigger(
			&(self->on_triggered), self->event_loop,
			(void*) aux_data
		);
	}
}
