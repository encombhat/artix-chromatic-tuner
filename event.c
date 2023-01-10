/*
 * event.c
 *
 *  Created on: Oct 22, 2022
 *      Author: bhat
 */

#include "event.h"

#include "mb_interface.h"

#include <stdbool.h>

struct EventLoop event_loop_new() {
	struct EventLoop event_loop = {
		.buf = {0},
		.buf_data_ptrs = {0},
		.buf_aux = {
			.size = EVENT_LOOP_BUF_SIZE,
			.head = 0, .tail = 0,
		},
	};

	return event_loop;
}

bool event_loop_empty(struct EventLoop* self) {
	return buf_empty(self->buf_aux);
}

// Can only be used in main loop!!!
// This is not interrupt safe
void event_loop_push(
	struct EventLoop* self,
	void (*slot)(void*, void*), void* data_ptr, void* aux_ptr
) {
	buf_push((u32*) self->buf, (u32) slot, self->buf_aux);
	buf_push((u32*) self->buf_data_ptrs, (u32) data_ptr, self->buf_aux);
	buf_push((u32*) self->buf_aux_ptrs, (u32) aux_ptr, self->buf_aux);
	bufaux_push(&(self->buf_aux));
}

void event_loop_execute_one(struct EventLoop* self) {
	void (*slot)(void*, void*) = (SLOT_TYPE) buf_pop(
		(u32*) self->buf, self->buf_aux
	);
	void* data_ptr = (void*) buf_pop(
		(u32*) self->buf_data_ptrs, self->buf_aux
	);
	void* aux_ptr = (void*) buf_pop(
		(u32*) self->buf_aux_ptrs, self->buf_aux
	);
	bufaux_pop(&(self->buf_aux));

	(*slot)(data_ptr, aux_ptr);
}

struct Signal signal_new() {
	struct Signal signal = {
		.slots = {0},
		.slot_count = 0,
	};

	return signal;
}

void signal_connect(
	struct Signal* self,
	void (*slot)(void*, void*), void* data_ptr
) {
	if (self->slot_count == SIGNAL_MAX_SLOT_COUNT) {
		xil_printf("signal slots is full...\r\n");
		return;
	}

	self->slots[self->slot_count] = slot;
	self->slot_data_ptrs[self->slot_count] = data_ptr;
	self->slot_count += 1;
}

void signal_disconnect(
	struct Signal* self,
	void (*slot)(void*, void*)
) {
	bool found = false;
	for (int i = 0; i < self->slot_count; i += 1) {
		if (found) {
			self->slots[i-1] = self->slots[i];
			self->slot_data_ptrs[i-1] = self->slot_data_ptrs[i];
			continue;
		}
		if (self->slots[i] == slot) {
			found = true;
			continue;
		}
	}

	if (!found) {
		xil_printf("slot disconnect failed: no such slot\r\n");
		return;
	}

	self->slot_count -= 1;
}

void signal_trigger(
	struct Signal* self, struct EventLoop* event_loop,
	void* aux_ptr
) {
	for (u32 i = 0; i < self->slot_count; i += 1) {
		void (*slot)(void*, void*) = self->slots[i];
		void* data_ptr = self->slot_data_ptrs[i];

		event_loop_push(event_loop, slot, data_ptr, aux_ptr);
	}
}
