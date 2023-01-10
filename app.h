/*
 * app.h
 *
 *  Created on: Oct 11, 2022
 *      Author: bhat
 */

#ifndef SRC_APP_H_
#define SRC_APP_H_

#include "event.h"

#include <stdlib.h>

#include "io/timer.h"
#include "io/mic.h"
#include "io/button.h"
#include "io/encoder.h"
#include "virtio/encoder.h"
#include "virtio/button.h"

#include "io/lcd.h"

#include "virtio/tone.h"
#include "virtio/periodic.h"
#include "virtio/prompt.h"

#include "main_page.h"
#include "chroma.h"
#include "hist.h"

struct App;
void app_initialize(struct App* self);
void app_main_loop(struct App* self);
void app_open_chroma(struct App* self);

enum AppState {
	App_Main,
	App_Chroma,
};

struct App {
	// Main event loop
	struct EventLoop event_loop;

	// States
	float f_s;
	enum AppState state;

	// Sensors
	struct Timer timer;
	struct Mic mic;
	struct Encoder encoder;
	struct Button btn;

	// Virtual sensors
	struct VirtEncoder virt_encoder;
	struct VirtButton encoder_btn;
	struct VirtButton btn_left;
	struct VirtButton btn_right;
	struct VirtButton btn_top;
	struct VirtButton btn_bottom;

	// Actuators
	struct Lcd lcd;

	// Virtual actuators
	struct MainPage* main_page;
	struct Chroma* chroma;
	struct Hist* hist;

	// Signals

};

struct App app_new(unsigned int tick);

#endif /* SRC_APP_H_ */
