/*
 * app.c
 *
 *  Created on: Oct 11, 2022
 *      Author: bhat
 */

#include "app.h"

#include "io/buf.h"

#include "hw_map.h"
#include "config.h"

#include "app_slots.h"

// Opportunistically runs while polling
void _app_idle(struct App* self);
// Only runs once per loop iteration
void _app_unassertive_tick(struct App* self);

struct App app_new(unsigned int tick) {
	float f_s = 100 * 1000 * 1000 / 2048.0;

	struct App app = {
		.event_loop = event_loop_new(),

		.f_s = f_s,
		.state = App_Main,

		.timer = timer_with_interval(tick, TIMER_ID, TIMER_INTR),
		.mic = mic_new(MIC_BASE_ADDR),
		.encoder = encoder_new(ENCODER_ID, ENCODER_INTR, 0),
		.btn = button_new(BTN_ID, BTN_INTR),

		.lcd = lcd_new(LCD_GPIO_ID, LCD_SPI_ID),
		.virt_encoder = virt_encoder_new(ENCODER_LATENCY / tick),
		.encoder_btn = virt_button_new(BTN_LATENCY / tick),
		.btn_left = virt_button_new(BTN_LATENCY / tick),
		.btn_right = virt_button_new(BTN_LATENCY / tick),
		.btn_top = virt_button_new(BTN_LATENCY / tick),
		.btn_bottom = virt_button_new(BTN_LATENCY / tick),

		.main_page = 0,
		.chroma = 0,
		.hist = 0,
	};

	return app;
}

bool _app_virt_btn_left_accessor(struct Button* btn) {
	u32 event_data = button_event_rt(btn);
	return (event_data & BTN_L_MASK) > 0;
}

bool _app_virt_btn_right_accessor(struct Button* btn) {
	u32 event_data = button_event_rt(btn);
	return (event_data & BTN_R_MASK) > 0;
}

bool _app_virt_btn_top_accessor(struct Button* btn) {
	u32 event_data = button_event_rt(btn);
	return (event_data & BTN_T_MASK) > 0;
}

bool _app_virt_btn_bottom_accessor(struct Button* btn) {
	u32 event_data = button_event_rt(btn);
	return (event_data & BTN_B_MASK) > 0;
}

void app_initialize(struct App* self) {
	self->encoder.ts_now_ptr = &(self->timer.count);
	struct EventLoop* event_loop = &(self->event_loop);

	// Must initialize everything before registering, as
	// XIntc_Initialize disables all interrupts
	timer_initialize(&(self->timer), event_loop);
	encoder_initialize(&(self->encoder), event_loop);
	button_initialize(&(self->btn), event_loop);

	// Now register
	timer_register(&(self->timer));
	encoder_register(&(self->encoder));
	button_register(&(self->btn));

	microblaze_enable_interrupts();

	mic_initialize(&(self->mic), event_loop, &(self->timer.count));
	virt_encoder_initialize(&(self->virt_encoder), event_loop);
	// This is dangerous shit
	virt_button_initialize(
		&(self->encoder_btn), event_loop,
		(VIRT_BUTTON_ACCESSOR_TYPE) encoder_pressed_rt, (void*) &(self->encoder)
	);
	virt_button_initialize(
		&(self->btn_left), event_loop,
		(VIRT_BUTTON_ACCESSOR_TYPE) _app_virt_btn_left_accessor, (void*) &(self->btn)
	);
	virt_button_initialize(
		&(self->btn_right), event_loop,
		(VIRT_BUTTON_ACCESSOR_TYPE) _app_virt_btn_right_accessor, (void*) &(self->btn)
	);
	virt_button_initialize(
		&(self->btn_top), event_loop,
		(VIRT_BUTTON_ACCESSOR_TYPE) _app_virt_btn_top_accessor, (void*) &(self->btn)
	);
	virt_button_initialize(
		&(self->btn_bottom), event_loop,
		(VIRT_BUTTON_ACCESSOR_TYPE) _app_virt_btn_bottom_accessor, (void*) &(self->btn)
	);

	lcd_initialize(&(self->lcd), event_loop);

	_app_connect_signal_slot(self);

	struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
	req->type = lcd_request_ClearAll;
	lcd_draw_async(&(self->lcd), req);

	self->main_page = main_page_new(self);
	main_page_initialize(self->main_page);

	xil_printf("app started, version=%d.%d.%d\r\n", 0, 1, 0);
}

void app_main_loop(struct App* self) {
	struct EventLoop* event_loop = &(self->event_loop);

	while (event_loop_empty(event_loop)) _app_idle(self);
	while (!event_loop_empty(event_loop)) { // Polling
		event_loop_execute_one(event_loop);
	}

	_app_unassertive_tick(self);
}

void _app_idle(struct App* self) {
	timer_idle(&(self->timer));
	encoder_idle(&(self->encoder));
	button_idle(&(self->btn));
	mic_idle(&(self->mic));
}

void _app_unassertive_tick(struct App* self) {
	u32 ts_now = timer_ts_now(&(self->timer));

	if (self->chroma) {
		chroma_tick(self->chroma, ts_now);
	}
	if (self->hist) {
		hist_tick(self->hist, ts_now);
	}
	virt_encoder_tick(&(self->virt_encoder), ts_now);
	virt_button_tick(&(self->encoder_btn), ts_now);
	virt_button_tick(&(self->btn_left), ts_now);
	virt_button_tick(&(self->btn_right), ts_now);
	virt_button_tick(&(self->btn_top), ts_now);
	virt_button_tick(&(self->btn_bottom), ts_now);
}
