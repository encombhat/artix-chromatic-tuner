/*
 * oneshot_led.h
 *
 *  Created on: Oct 21, 2022
 *      Author: bhat
 */

#ifndef SRC_VIRTIO_ONESHOT_H_
#define SRC_VIRTIO_ONESHOT_H_

#include "../event.h"

#include <stdbool.h>

#include "xil_types.h"

struct VirtOneshot;
void virt_oneshot_initialize(
	struct VirtOneshot* self,
	struct EventLoop* event_loop
);
void virt_oneshot_trigger(
	struct VirtOneshot* self,
	u32 ts_now
);
void virt_oneshot_reset(
	struct VirtOneshot* self
);
void virt_oneshot_tick(
	struct VirtOneshot* self,
	u32 ts_now
);

enum VirtOneshotState {
	virt_oneshot_Off,
	virt_oneshot_Running,
};

struct VirtOneshot {
	struct EventLoop* event_loop;

	u32 duration;

	enum VirtOneshotState state;
	u32 event_ts;

	bool triggered;

	// Signals
	struct Signal on_triggered;
};

struct VirtOneshot virt_oneshot_new(
	u32 period
);

#endif /* SRC_VIRTIO_ONESHOT_H_ */
