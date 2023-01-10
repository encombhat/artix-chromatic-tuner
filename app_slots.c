/*
 * app_slots.c
 *
 *  Created on: Oct 22, 2022
 *      Author: bhat
 */

#include "app_slots.h"

#include "hw_map.h"
#include "app.h"

#include <stdlib.h>
#include <math.h>

void _app_timer_slot(struct App* self, u32 count) {
//	xil_printf("Timer triggered!!!\r\n");
}

void _app_encoder_slot(struct App* self, u32 aux_data) {
	u32 event_data = aux_data & 0xF;
	u32 event_ts = aux_data >> 4;
//	xil_printf("encoder event occurred: %d at %d\r\n", event_data, event_ts);

	virt_encoder_handle_event(
		&(self->virt_encoder),
		event_data & 0x3 /* 0b11 */, event_ts
	);

	if ((event_data & 0x4) > 0) {
		virt_button_trigger(
			&(self->encoder_btn), timer_ts_now(&(self->timer))
		);
	}
}

void _app_btn_slot(struct App* self, u32 event_data) {
	if ((event_data & BTN_L_MASK) > 0) {
		virt_button_trigger(
			&(self->btn_left), timer_ts_now(&(self->timer))
		);
	}
	if ((event_data & BTN_R_MASK) > 0) {
		virt_button_trigger(
			&(self->btn_right), timer_ts_now(&(self->timer))
		);
	}
	if ((event_data & BTN_T_MASK) > 0) {
		virt_button_trigger(
			&(self->btn_top), timer_ts_now(&(self->timer))
		);
	}
	if ((event_data & BTN_B_MASK) > 0) {
		virt_button_trigger(
			&(self->btn_bottom), timer_ts_now(&(self->timer))
		);
	}
}

void _app_btn_left_triggered_slot(struct App* self, void* aux_data) {
	// Chromatic tuner
	if (self->chroma) return;

	if (self->main_page) {
		main_page_destroy(self->main_page);
		free(self->main_page);
		self->main_page = 0;
	}

	if (self->hist) {
		hist_destroy(self->hist);
		free(self->hist);
		self->hist = 0;
	}

	self->chroma = chroma_new(self);
	chroma_initialize(self->chroma);
}

void _app_btn_right_triggered_slot(struct App* self, void* aux_data) {
	if (self->hist) return;

	if (self->chroma) {
		chroma_destroy(self->chroma);
		free(self->chroma);
		self->chroma = 0;
	}

	if (self->main_page) {
		main_page_destroy(self->main_page);
		free(self->main_page);
		self->main_page = 0;
	}

	self->hist = hist_new(self);
	hist_initialize(self->hist);
}

void _app_btn_bottom_triggered_slot(struct App* self, void* aux_data) {
	if (self->main_page) return;

	if (self->chroma) {
		chroma_destroy(self->chroma);
		free(self->chroma);
		self->chroma = 0;
	}

	if (self->hist) {
		hist_destroy(self->hist);
		free(self->hist);
		self->hist = 0;
	}

	self->main_page = main_page_new(self);
	main_page_initialize(self->main_page);
}

void _app_connect_signal_slot(struct App* self) {
	signal_connect(
		&(self->timer.on_triggered),
		(SLOT_TYPE) &_app_timer_slot, (void*) self
	);

	signal_connect(
		&(self->encoder.on_triggered),
		(SLOT_TYPE) &_app_encoder_slot, (void*) self
	);
	signal_connect(
		&(self->btn.on_triggered),
		(SLOT_TYPE) &_app_btn_slot, (void*) self
	);

	signal_connect(
		&(self->btn_left.on_triggered),
		(SLOT_TYPE) &_app_btn_left_triggered_slot, (void*) self
	);
	signal_connect(
		&(self->btn_right.on_triggered),
		(SLOT_TYPE) &_app_btn_right_triggered_slot, (void*) self
	);
	signal_connect(
		&(self->btn_bottom.on_triggered),
		(SLOT_TYPE) &_app_btn_bottom_triggered_slot, (void*) self
	);
}
