/*
 * chroma.c
 *
 *  Created on: Dec 1, 2022
 *      Author: bhat
 */

#include "chroma.h"

struct Chroma* chroma_new(struct App* app) {
	struct Chroma* chroma = malloc(sizeof(struct Chroma));
	chroma->app = app;
	chroma->state = Chroma_Nominal;
	chroma->set_octave = 2;
	chroma->timer_encoder = virt_oneshot_new(2000);
	chroma->sign = sign_new();
	chroma->dev_bar = hori_bar_new();
	chroma->label = chroma_label_new(
		&app->lcd,
		110, 160, 230, 200,
		"f:", 24, "Hz", 24
	);
	chroma->a4_label = chroma_label_new(
		&app->lcd,
		110, 210, 230, 250,
		"A4:", 30, "Hz", 24
	);
	chroma_label_set_state(&(chroma->a4_label), 440);

	chroma->tone = virt_tone_new(app->f_s / 4.f, 40);

	return chroma;
}

void _chroma_set_state(struct Chroma* self, enum ChromaState state) {
	if (self->state == Chroma_A4 && state == Chroma_A4) {
		chroma_label_refresh(&(self->a4_label));

		return;
	}

	if (self->state == state) return;

	if (self->state == Chroma_A4) { // exiting A4
		chroma_label_hide(&(self->a4_label));
		virt_oneshot_reset(&(self->timer_encoder));
	}

	self->state = state;

	if (self->state == Chroma_A4) { // entering A4
		chroma_label_show(&(self->a4_label));
	}

	self->sign.highlight = (self->state == Chroma_Octave);
	if (self->state == Chroma_Octave) {
		self->sign.octave = self->set_octave;
		self->sign.note = 0;
	}

	sign_show(&(self->sign));
}

void _chroma_encoder_cw_slot(struct Chroma* self) {
	if (self->state == Chroma_Octave) {
		if (self->set_octave < 8) {
			self->set_octave += 1;
		}
		sign_set_states(&(self->sign), self->set_octave, 0);
	} else {
		virt_oneshot_trigger(&(self->timer_encoder),
				timer_ts_now(&(self->app->timer)));

		if (self->a4_label.state < 460) {
			chroma_label_set_state(&(self->a4_label), self->a4_label.state + 1);
		}
		_chroma_set_state(self, Chroma_A4);
	}
}

void _chroma_encoder_ccw_slot(struct Chroma* self) {
	if (self->state == Chroma_Octave) {
		if (self->set_octave > 2) {
			self->set_octave -= 1;
		}
		sign_set_states(&(self->sign), self->set_octave, 0);
	} else {
		virt_oneshot_trigger(&(self->timer_encoder),
				timer_ts_now(&(self->app->timer)));

		if (self->a4_label.state > 420) {
			chroma_label_set_state(&(self->a4_label), self->a4_label.state - 1);
		}

		_chroma_set_state(self, Chroma_A4);
	}
}

void _chroma_encoder_btn_slot(struct Chroma* self) {
	if (self->state == Chroma_Octave) {
		_chroma_set_state(self, Chroma_Nominal);
	} else {
		_chroma_set_state(self, Chroma_Octave);
	}
}

void _chroma_timer_encoder_slot(struct Chroma* self) {
	_chroma_set_state(self, Chroma_Nominal);
}

static short chroma_mic_buf[1024];
static short buf_aux[1024];

void _chroma_mic_slot(struct Chroma* self, short* mic_buf) {
	if (self->state == Chroma_Octave && self->set_octave < 2) return;

	u32 freq;
	if (self->state == Chroma_Octave) {
		u32 res = virt_tone_identify_with_octave(
			&(self->tone),
			self->a4_label.state, self->set_octave,
			mic_buf, buf_aux
		);
		freq = (res >> 16);
	} else {
		freq = virt_tone_max_freq(
			&(self->tone),
			mic_buf, buf_aux
		);
	}

	if (freq < 1) return;
//	xil_printf("highest freq: %d\r\n", freq);

	u32 merge_count;
	if (freq < self->app->f_s / 256) {
		merge_count = 16;
	} else if (freq < self->app->f_s / 64) {
		merge_count = 8;
	} else if (freq < self->app->f_s / 16) {
		merge_count = 4;
	} else if (freq < self->app->f_s / 8) {
		merge_count = 4;
	} else {
		merge_count = 2;
	}

	mic_fill_async(
		&(self->app->mic),
		chroma_mic_buf,
		merge_count
	);
}

void _chroma_mic_mate_slot(struct Chroma* self, u32 merge_count) {
	if (self->state == Chroma_Octave && self->set_octave < 2) return;

	float f_s = self->app->f_s / merge_count;

//	xil_printf("[");
//	for (u32 i = 0; i < 100; i += 1) {
//		xil_printf("%d, ", chroma_mic_buf[i]);
//	}
//	xil_printf("]\r\n");

	struct VirtTone virt_tone = virt_tone_new(f_s, 40);
	u32 new_res;
	if (self->state == Chroma_Octave) {
		new_res = virt_tone_identify_with_octave(
			&virt_tone,
			self->a4_label.state, self->set_octave,
			chroma_mic_buf, buf_aux
		);
	} else {
		new_res = virt_tone_identify(
			&virt_tone,
			self->a4_label.state,
			chroma_mic_buf, buf_aux
		);
	}

//	xil_printf("mate called: %d\r\n", new_res);

	if (new_res < 1) return;

	u32 new_freq = (new_res >> 16);
	u32 new_deviation = (new_res >> 8) & 0xFF;
	u32 new_octave = (new_res >> 4) & 0xF;
	u32 new_tone = new_res & 0xF;

	if (self->state == Chroma_Octave && self->set_octave != new_octave) return;

	sign_set_states(&(self->sign), new_octave, new_tone);
	hori_bar_set_state(&(self->dev_bar), new_deviation);
	chroma_label_set_state(&(self->label), new_freq);
	chroma_label_refresh(&(self->label));
}


void chroma_initialize(
	struct Chroma* self
) {
	virt_oneshot_initialize(&(self->timer_encoder), &(self->app->event_loop));

	sign_initialize(&(self->sign), &(self->app->lcd));
	hori_bar_initialize(&(self->dev_bar), &(self->app->lcd));
	chroma_label_initialize(&(self->label));

	sign_show(&(self->sign));
	hori_bar_show(&(self->dev_bar));
	chroma_label_show(&(self->label));

	signal_connect(
		&(self->app->virt_encoder.on_clockwise),
		(SLOT_TYPE) &_chroma_encoder_cw_slot, (void*) self
	);
	signal_connect(
		&(self->app->virt_encoder.on_counterclockwise),
		(SLOT_TYPE) &_chroma_encoder_ccw_slot, (void*) self
	);
	signal_connect(
		&(self->app->encoder_btn.on_triggered),
		(SLOT_TYPE) &_chroma_encoder_btn_slot, (void*) self
	);
	signal_connect(
		&(self->timer_encoder.on_triggered),
		(SLOT_TYPE) &_chroma_timer_encoder_slot, (void*) self
	);

	signal_connect(
		&(self->app->mic.on_check),
		(SLOT_TYPE) &_chroma_mic_slot, (void*) self
	);
	signal_connect(
		&(self->app->mic.on_mate),
		(SLOT_TYPE) &_chroma_mic_mate_slot, (void*) self
	);
}

void chroma_destroy(
	struct Chroma* self
) {
	signal_disconnect(
		&(self->app->virt_encoder.on_clockwise),
		(SLOT_TYPE) &_chroma_encoder_cw_slot
	);
	signal_disconnect(
		&(self->app->virt_encoder.on_counterclockwise),
		(SLOT_TYPE) &_chroma_encoder_ccw_slot
	);
	signal_disconnect(
		&(self->app->encoder_btn.on_triggered),
		(SLOT_TYPE) &_chroma_encoder_btn_slot
	);
	signal_disconnect(
		&(self->timer_encoder.on_triggered),
		(SLOT_TYPE) &_chroma_timer_encoder_slot
	);
	signal_disconnect(
		&(self->app->mic.on_check),
		(SLOT_TYPE) &_chroma_mic_slot
	);
	signal_disconnect(
		&(self->app->mic.on_mate),
		(SLOT_TYPE) &_chroma_mic_mate_slot
	);

	sign_hide(&(self->sign));
	hori_bar_hide(&(self->dev_bar));
	chroma_label_hide(&(self->label));
	chroma_label_hide(&(self->a4_label));
}

void chroma_tick(
	struct Chroma* self,
	u32 ts_now
) {
	virt_oneshot_tick(&(self->timer_encoder), ts_now);
}
