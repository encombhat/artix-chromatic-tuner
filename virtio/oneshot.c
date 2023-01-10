/*
 * oneshot_led.c
 *
 *  Created on: Oct 21, 2022
 *      Author: bhat
 */

#include "oneshot.h"

struct VirtOneshot virt_oneshot_new(
	u32 duration
) {
	struct VirtOneshot oneshot = {
		.duration = duration,

		.state = virt_oneshot_Off,

		.triggered = false,
	};

	return oneshot;
}

void virt_oneshot_initialize(
	struct VirtOneshot* self,
	struct EventLoop* event_loop
) {
	self->event_loop = event_loop;
}

void virt_oneshot_trigger(
	struct VirtOneshot* self,
	u32 ts_now
) {
	self->event_ts = ts_now;
	self->state = virt_oneshot_Running;
}

void virt_oneshot_reset(
	struct VirtOneshot* self
) {
	self->event_ts = 0;
	self->state = virt_oneshot_Off;
}

void virt_oneshot_tick(
	struct VirtOneshot* self,
	u32 ts_now
) {
	if (self->state == virt_oneshot_Off) return; // timer is off

	if (ts_now - self->event_ts > self->duration) {
		// Expired
//		xil_printf("timer expired...\r\n");
		self->state = virt_oneshot_Off;

		signal_trigger(&(self->on_triggered), self->event_loop, (void*) ts_now);
	}
}
