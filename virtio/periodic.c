/*
 * blink_led.c
 *
 *  Created on: Oct 20, 2022
 *      Author: bhat
 */

#include "periodic.h"

struct VirtPeriodic virt_periodic_new(
	u32 period
) {
	struct VirtPeriodic led = {
		.event_loop = 0,

		.period = period,

		.state = virt_periodic_Off,

		.on_triggered = signal_new(),
	};

	return led;
}

void virt_periodic_initialize(
	struct VirtPeriodic* self,
	struct EventLoop* event_loop
) {
	self->event_loop = event_loop;
}

void _virt_periodic_update_state(
	struct VirtPeriodic* self,
	enum VirtPeriodicState new_state
) {
	if (new_state == self->state) return;
	self->state = new_state;

	if (new_state == virt_periodic_Off) return;

	signal_trigger(
		&(self->on_triggered),
		self->event_loop, (void*) 0
	);
}

void virt_periodic_tick(
	struct VirtPeriodic* self,
	u32 ts_now
) {
	if (ts_now % self->period == 0) {
		_virt_periodic_update_state(self, virt_periodic_Triggering);
	} else {
		_virt_periodic_update_state(self, virt_periodic_Off);
	}
}
