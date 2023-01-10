/*
 * event.h
 *
 *  Created on: Oct 22, 2022
 *      Author: bhat
 */

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#define SIGNAL_MAX_SLOT_COUNT 4

#include "io/buf.h"

#include <stdbool.h>

#include "xil_types.h"

#define SLOT_TYPE void (*)(void*, void*)
#define SLOT_VAR(var_name) void (* ## var_name ## )(void*, void*)

struct EventLoop;
bool event_loop_empty(struct EventLoop* self);
void event_loop_push(
	struct EventLoop* self,
	SLOT_TYPE, void* data_ptr, void* aux_ptr
);
void event_loop_execute_one(struct EventLoop* self);

struct Signal;
void signal_initialize(
	struct Signal* self,
	struct EventLoop* event_loop
);
void signal_connect(
	struct Signal* self,
	SLOT_TYPE, void* data_ptr
);
void signal_disconnect(
	struct Signal* self,
	SLOT_TYPE
);
void signal_trigger(
	struct Signal* self, struct EventLoop* event_loop,
	void* aux_ptr
);

#define EVENT_LOOP_BUF_SIZE 128
struct EventLoop {
	void (*buf[EVENT_LOOP_BUF_SIZE]) (void*, void*);
	void* buf_data_ptrs[EVENT_LOOP_BUF_SIZE];
	void* buf_aux_ptrs[EVENT_LOOP_BUF_SIZE];
	struct BufAux buf_aux;
};
struct EventLoop event_loop_new();

struct Signal {
	void (*slots[SIGNAL_MAX_SLOT_COUNT])(void*, void*);
	void* slot_data_ptrs[SIGNAL_MAX_SLOT_COUNT];
	u32 slot_count;
};
struct Signal signal_new();

#endif /* SRC_EVENT_H_ */
