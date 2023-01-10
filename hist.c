/*
 * hist.c
 *
 *  Created on: Dec 2, 2022
 *      Author: bhat
 */

#include "hist.h"

static float tone_rectifier[1024];

struct Hist* hist_new(struct App* app) {
	memset(tone_rectifier, 0, sizeof tone_rectifier);
	for (int i = 3; i < 600; i += 1) {
		tone_rectifier[i] = 1.f;
	}
	for (int i = 3; i < 12; i += 1) {
		tone_rectifier[i] = (float) i / 12.f;
	}

	struct Hist* hist = malloc(sizeof(struct Hist));

	hist->app = app;
	hist->tone = virt_tone_new(app->f_s / 4.f, 40);
	hist->periodic = virt_periodic_new(200);

	hist->freq_label = chroma_label_new(
		&app->lcd,
		110, 10, 230, 50,
		"f:", 24, "Hz", 24
	);
	hist->spacing_label = chroma_label_new(
		&app->lcd,
		110, 60, 230, 100,
		"s:", 24, "Hz", 24
	);
	chroma_label_set_state(&(hist->spacing_label), app->f_s / 4096.f);

	return hist;
}

#define HIST_X1 10
#define HIST_Y1 160
#define HIST_X2 230
#define HIST_Y2 310

void _hist_draw_background(struct Hist* self) {
	struct Lcd* lcd = &(self->app->lcd);

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Dialog;
		req->x1 = HIST_X1;
		req->y1 = HIST_Y1;
		req->x2 = HIST_X2;
		req->y2 = HIST_Y2;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		lcd_draw_async(lcd, req);
	}
}

static short buf_aux[1024];
static u8 buf_hist[128];

void _hist_mic_slot(struct Hist* self, short* mic_buf) {
	u32 freq = virt_tone_max_freq(
		&(self->tone),
		mic_buf, buf_aux
	);
//	xil_printf("hist:freq %d\r\n", freq);

	chroma_label_set_state(&(self->freq_label), freq);
	chroma_label_refresh(&(self->freq_label));
}

void _hist_periodic_slot(struct Hist* self, void* aux_ptr) {
	for (u32 i = 0; i < 128; i += 1) {
		double res = 0.f;
		for (u32 k = 0; k < 4; k += 1) {
			res += buf_aux[i * 4 + k];
		}

		res = log(res);
		res *= 24.f;

		if (res > 255.f) {
			buf_hist[i] = 255;
		} else if (res < 0.f) {
			buf_hist[i] = 0;
		} else {
			buf_hist[i] = res;
		}
	}

	struct Lcd* lcd = &(self->app->lcd);
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Histogram;
		req->x1 = HIST_X1 + 10;
		req->y1 = HIST_Y1 + 10;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = buf_hist;

		lcd_draw_async(lcd, req);
	}
}

void hist_initialize(struct Hist* self) {
	virt_periodic_initialize(&(self->periodic), &(self->app->event_loop));
	chroma_label_initialize(&(self->freq_label));
	chroma_label_initialize(&(self->spacing_label));

	chroma_label_show(&(self->freq_label));
	chroma_label_show(&(self->spacing_label));

	signal_connect(
		&(self->app->mic.on_check),
		(SLOT_TYPE) &_hist_mic_slot, (void*) self
	);
	signal_connect(
		&(self->periodic.on_triggered),
		(SLOT_TYPE) &_hist_periodic_slot, (void*) self
	);

	_hist_draw_background(self);

}

void hist_destroy(struct Hist* self) {
	signal_disconnect(
		&(self->app->mic.on_check),
		(SLOT_TYPE) &_hist_mic_slot
	);
	signal_disconnect(
		&(self->periodic.on_triggered),
		(SLOT_TYPE) &_hist_periodic_slot
	);

	chroma_label_hide(&(self->freq_label));
	chroma_label_hide(&(self->spacing_label));

	struct Lcd* lcd = &(self->app->lcd);
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Clear;
		req->x1 = HIST_X1;
		req->y1 = HIST_Y1;
		req->x2 = HIST_X2;
		req->y2 = HIST_Y2;

		lcd_draw_async(lcd, req);
	}
}

void hist_tick(
	struct Hist* self,
	u32 ts_now
) {
	virt_periodic_tick(&(self->periodic), ts_now);
}
