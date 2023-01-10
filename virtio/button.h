/*
 * button.h
 *
 *  Created on: Oct 29, 2022
 *      Author: bhat
 */

#ifndef SRC_VIRTIO_BUTTON_H_
#define SRC_VIRTIO_BUTTON_H_

#include "oneshot.h"

#include "../event.h"

#include "xil_types.h"

#include <stdbool.h>

#define VIRT_BUTTON_ACCESSOR_TYPE bool (*)(void*)

struct VirtButton;
void virt_button_initialize(
	struct VirtButton* self,
	struct EventLoop* event_loop,
	bool (*accessor)(void*),
	void* accessor_aux_ptr
);
void virt_button_trigger(
	struct VirtButton* self,
	u32 ts_now
);
void virt_button_tick(
	struct VirtButton* self,
	u32 ts_now
);

struct VirtButton {
	struct EventLoop* event_loop;
	bool (*accessor)(void*);
	void* accessor_aux_ptr;

	struct VirtOneshot oneshot;

	// Signals
	struct Signal on_triggered;
};
struct VirtButton virt_button_new(u32 latency);

#endif /* SRC_VIRTIO_BUTTON_H_ */
