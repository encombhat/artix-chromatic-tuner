/*
 * button.h
 *
 *  Created on: Oct 29, 2022
 *      Author: bhat
 */

#ifndef SRC_IO_BUTTON_H_
#define SRC_IO_BUTTON_H_

#include "buf.h"

#include "interrupt.h"
#include "xgpio.h"

#include "../event.h"

struct Button;
void button_initialize(
	struct Button* self,
	struct EventLoop* event_loop
);
void button_register(struct Button* self);
u32 button_event_rt(struct Button* self);

void button_idle(struct Button* self);

#define BUTTON_EVENT_BUF_SIZE 32

struct Button {
	struct EventLoop* event_loop;

	struct IntrDev intrdev;

	u32 buf[BUTTON_EVENT_BUF_SIZE];
	struct BufAux buf_aux;

	struct IntrCon intrcon;
	XGpio sys_gpio;

	// Signals
	struct Signal on_triggered;
};
struct Button button_new(
	u8 device_id,
	u8 intr_id
);

#endif /* SRC_IO_BUTTON_H_ */
