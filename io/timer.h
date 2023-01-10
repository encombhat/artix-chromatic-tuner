/*
 * timer.h
 *
 *  Created on: Oct 10, 2022
 *      Author: bhat
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include "interrupt.h"
#include "../event.h"

#include <stdbool.h>

#include "xtmrctr.h"

struct Timer;
void timer_initialize(
	struct Timer* self,
	struct EventLoop* event_loop
);
void timer_register(struct Timer* self);

u32 timer_ts_now(struct Timer* self);
void timer_reset(struct Timer* self);

void timer_idle(struct Timer* self);

struct Timer {
	struct EventLoop* event_loop;

	struct IntrDev intrdev;
	unsigned int reset_value;

	u32 count;
	u32 count_head;

	struct IntrCon intrcon;
	XTmrCtr sys_tmrctr;

	// Signals
	struct Signal on_triggered;
};

struct Timer timer_with_interval(
	unsigned int interval,
	u8 device_id, u8 intr_id
);
struct Timer timer_new(
	u8 device_id, u8 intr_id
);

#endif /* SRC_TIMER_H_ */
