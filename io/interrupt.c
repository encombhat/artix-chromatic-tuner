/*
 * interrupt.c
 *
 *  Created on: Oct 10, 2022
 *      Author: bhat
 */

#include "interrupt.h"

struct IntrCon intrcon_new()  {
	// 0 is the id for the first interrupt controller
	return intrcon_with_device_id(0);
}

struct IntrCon intrcon_with_device_id(u8 device_id) {
	struct IntrCon intrcon = {
		.device_id = device_id,
		.sys_intc = {},
	};

	return intrcon;
}

void intrcon_initialize(struct IntrCon* intrcon) {
	XStatus _status = XIntc_Initialize(
		&(intrcon->sys_intc),
		intrcon->device_id
	);

	if (_status != XST_SUCCESS) {
		if(_status == XST_DEVICE_NOT_FOUND) {
			xil_printf("XST_DEVICE_NOT_FOUND...\r\n");
		} else {
			xil_printf("a different error from XST_DEVICE_NOT_FOUND...\r\n");
		}

		xil_printf("Interrupt controller driver failed to be initialized...\r\n");
		return;
	}

//	xil_printf("Interrupt controller driver initialized!\r\n");
}

void intrcon_register_device(
	struct IntrCon* intrcon,
	struct IntrDev* intr_dev,
	XInterruptHandler handler,
	void* callback_ref
) {
	{
		XStatus _status = XIntc_Connect(
				&(intrcon->sys_intc),
				intr_dev->intr_id, handler, callback_ref);
		if (_status != XST_SUCCESS) {
			xil_printf("Failed to connect the application handlers to the interrupt controller...\r\n");
			return;
		}
	}

	XIntc_Enable(&(intrcon->sys_intc), intr_dev->intr_id);

//	xil_printf("Connected %d to Interrupt Controller!\r\n", intr_dev->intr_id);
}

void intrcon_start_controller(struct IntrCon* intrcon) {
	{
		XStatus _status = XIntc_Start(&(intrcon->sys_intc), XIN_REAL_MODE);
		if (_status != XST_SUCCESS) {
			xil_printf("Interrupt controller driver failed to start...\r\n");
			return;
		}
	}

//	xil_printf("Started Interrupt Controller!\r\n");
}


