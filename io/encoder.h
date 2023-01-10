/*
 * encoder.h
 *
 *  Created on: Oct 19, 2022
 *      Author: bhat
 */

#ifndef SRC_IO_ENCODER_H_
#define SRC_IO_ENCODER_H_

#include "buf.h"
#include "interrupt.h"
#include "../event.h"

#include <stdbool.h>

#include "xgpio.h"
#include "xil_types.h"

struct Encoder;
void encoder_initialize(
	struct Encoder* self,
	struct EventLoop* event_loop
);
void encoder_register(struct Encoder* self);
bool encoder_pressed_rt(struct Encoder* self);

void encoder_idle(struct Encoder* self);

#define ENCODER_EVENT_BUF_SIZE 32

struct Encoder {
	struct EventLoop* event_loop;

	struct IntrDev intrdev;
	u32* ts_now_ptr;

	u32 buf[ENCODER_EVENT_BUF_SIZE];
	struct BufAux buf_aux;
	u32 last_event;

	struct IntrCon intrcon;
	XGpio sys_gpio;

	// Signals
	struct Signal on_triggered;
};

struct Encoder encoder_new(
	u8 device_id, u8 intr_id,
	u32* ts_now_ptr
);

#endif /* SRC_IO_ENCODER_H_ */
