/*
 * button.c
 *
 *  Created on: Oct 29, 2022
 *      Author: bhat
 */

#include "button.h"

struct VirtButton virt_button_new(u32 latency) {
	struct VirtButton btn = {
		.event_loop = 0,

		.oneshot = virt_oneshot_new(latency),

		.on_triggered = signal_new(),
	};

	return btn;
}

void _virt_button_timer_triggered_slot(
	struct VirtButton* self,
	u32 ts_now
) {
	if ((self->accessor)(self->accessor_aux_ptr)) {
		signal_trigger(
			&(self->on_triggered),
			self->event_loop, 0
		);
	}
}

void virt_button_initialize(
	struct VirtButton* self,
	struct EventLoop* event_loop,
	bool (*accessor)(void*),
	void* accessor_aux_ptr
) {
	self->event_loop = event_loop;

	self->accessor = accessor;
	self->accessor_aux_ptr = accessor_aux_ptr;

	virt_oneshot_initialize(&(self->oneshot), event_loop);

	signal_connect(
		&(self->oneshot.on_triggered),
		(SLOT_TYPE) &_virt_button_timer_triggered_slot, (void*) self
	);
}

void virt_button_trigger(
	struct VirtButton* self,
	u32 ts_now
) {
	virt_oneshot_trigger(
		&(self->oneshot), ts_now
	);
}

void virt_button_tick(
	struct VirtButton* self,
	u32 ts_now
) {
	virt_oneshot_tick(&(self->oneshot), ts_now);
}
