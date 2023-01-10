/*
 * interrupt.h
 *
 *  Created on: Oct 10, 2022
 *      Author: bhat
 */

#ifndef SRC_INTERRUPT_H_
#define SRC_INTERRUPT_H_

#include "xintc.h"

struct IntrCon {
	u8 device_id;

	XIntc sys_intc;
};

struct IntrDev {
	u8 device_id;
	u8 intr_id;
};

struct IntrCon intrcon_new();
struct IntrCon intrcon_with_device_id(u8 device_id);
void intrcon_initialize(struct IntrCon* intrcon);

void intrcon_register_device(
	struct IntrCon* intrcon,
	struct IntrDev* intr_dev,
	XInterruptHandler handler,
	void* callback_ref
);

void intrcon_start_controller(struct IntrCon* intr);

#endif /* SRC_INTERRUPT_H_ */
