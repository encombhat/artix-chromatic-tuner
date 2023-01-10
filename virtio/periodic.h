/*
 * blink_led.h
 *
 *  Created on: Oct 20, 2022
 *      Author: bhat
 */

#ifndef SRC_VIRTIO_PERIODIC_H_
#define SRC_VIRTIO_PERIODIC_H_

#include "../event.h"

#include <stdbool.h>

#include "xil_types.h"

struct VirtPeriodic;
void virt_periodic_initialize(
	struct VirtPeriodic* self,
	struct EventLoop* event_loop
);
void virt_periodic_tick(
	struct VirtPeriodic* self,
	u32 ts_now
);

enum VirtPeriodicState {
	virt_periodic_Off,
	virt_periodic_Triggering,
};

struct VirtPeriodic {
	struct EventLoop* event_loop;

	u32 period;

	enum VirtPeriodicState state;

	// Signals
	struct Signal on_triggered;
};

struct VirtPeriodic virt_periodic_new(u32 period);

#endif /* SRC_VIRTIO_PERIODIC_H_ */
