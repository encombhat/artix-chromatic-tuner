/*
 * timer.c
 *
 *  Created on: Oct 10, 2022
 *      Author: bhat
 */

#include "timer.h"

#include "xil_types.h"

void _timer_handler(struct Timer* self);

struct Timer timer_with_interval(
	unsigned int interval,
	u8 device_id, u8 intr_id
) {
	struct Timer timer = {
		.event_loop = 0,

		.intrdev = {
			.device_id = device_id,
			.intr_id = intr_id,
		},
		.reset_value = interval,

		.count = 0,
		.count_head = 0,

		.intrcon = intrcon_new(),
		.sys_tmrctr = {},

		.on_triggered = signal_new(),
	};

	return timer;
}

struct Timer timer_new(
	u8 device_id, u8 intr_id
) {
	return timer_with_interval(
		1000,
		device_id,
		intr_id
	);
}

void timer_initialize(
	struct Timer* self,
	struct EventLoop* event_loop
) {
	intrcon_initialize(&(self->intrcon));

	{
		XStatus _status = XTmrCtr_Initialize(
			&(self->sys_tmrctr),
			self->intrdev.device_id
		);
		if (_status != XST_SUCCESS) {
			xil_printf("Timer initialization failed...\r\n");
			return;
		}
	}
	XTmrCtr_SetOptions(&(self->sys_tmrctr), 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetResetValue(&(self->sys_tmrctr), 0, 0xFFFFFFFF-self->reset_value);
	XTmrCtr_Start(&(self->sys_tmrctr), 0);

	self->event_loop = event_loop;
}

void _timer_handler(struct Timer* self) {
//	xil_printf("Timer interrupt occurred!!!");

	u32 ControlStatusReg = XTimerCtr_ReadReg(
			self->sys_tmrctr.BaseAddress, 0, XTC_TCSR_OFFSET);

	self->count += 1;

	// Profiler?
//	if (self->count % 1000 == 0) {
//		u32 ret_addr;
//		asm("add %0, r0, r14" : " = r"(ret_addr));
//		xil_printf("%x\r\n", ret_addr);
//	}

	XTmrCtr_WriteReg(self->sys_tmrctr.BaseAddress, 0, XTC_TCSR_OFFSET, ControlStatusReg |XTC_CSR_INT_OCCURED_MASK);
}

void timer_register(struct Timer* self) {
	intrcon_register_device(
		&(self->intrcon),
		&(self->intrdev),
		(XInterruptHandler)_timer_handler,
		(void*)self
	);
	intrcon_start_controller(&(self->intrcon));

	microblaze_register_handler(
		(XInterruptHandler)XIntc_DeviceInterruptHandler,
		(void*)self->intrdev.device_id
	);

	timer_reset(self);
}

u32 timer_ts_now(struct Timer* self) {
	return self->count;
}

void timer_reset(struct Timer* self) {
	XTmrCtr_SetControlStatusReg(self->sys_tmrctr.BaseAddress, 1, 0);
	XTmrCtr_SetLoadReg(self->sys_tmrctr.BaseAddress, 1, 0);
	XTmrCtr_SetControlStatusReg(self->sys_tmrctr.BaseAddress, 1, XTC_CSR_LOAD_MASK);
	XTmrCtr_SetControlStatusReg(self->sys_tmrctr.BaseAddress, 1, XTC_CSR_ENABLE_TMR_MASK);
}

void timer_idle(struct Timer* self) {
	if (self->count_head == self->count) return;
	self->count_head = self->count;

	signal_trigger(
		&(self->on_triggered),
		self->event_loop, (void*) self->count
	);
}
