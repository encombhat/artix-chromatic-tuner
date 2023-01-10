/*
 * hori_bar.c
 *
 *  Created on: Dec 2, 2022
 *      Author: bhat
 */

#include "hori_bar.h"

struct HoriBar hori_bar_new() {
	struct HoriBar bar = {
		.state = 50,
	};
	return bar;
}

void hori_bar_initialize(
	struct HoriBar* self,
	struct Lcd* lcd
) {
	self->lcd = lcd;
}

#define HORI_BAR_X1 110
#define HORI_BAR_Y1 130
#define HORI_BAR_X2 230
#define HORI_BAR_Y2 150

#define HORI_BAR_BAR_X1 120
#define HORI_BAR_BAR_Y1 137
#define HORI_BAR_BAR_X2 220
#define HORI_BAR_BAR_Y2 143

void _hori_bar_show_bar(struct HoriBar* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Rect;
		req->x1 = HORI_BAR_BAR_X1;
		req->y1 = HORI_BAR_BAR_Y1;
		req->x2 = HORI_BAR_BAR_X2;
		req->y2 = HORI_BAR_BAR_Y2;

		req->pix.r = 100;
		req->pix.g = 100;
		req->pix.b = 100;

		lcd_draw_async(self->lcd, req);
	}

	if (self->state < 50) {
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Rect;
		req->x1 = HORI_BAR_BAR_X1 + self->state;
		req->y1 = HORI_BAR_BAR_Y1;
		req->x2 = HORI_BAR_BAR_X1 + 50;
		req->y2 = HORI_BAR_BAR_Y2;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;

		lcd_draw_async(self->lcd, req);
	} else if (self->state > 50) {
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Rect;
		req->x1 = HORI_BAR_BAR_X1 + 50;
		req->y1 = HORI_BAR_BAR_Y1;
		req->x2 = HORI_BAR_BAR_X1 + self->state;
		req->y2 = HORI_BAR_BAR_Y2;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;

		lcd_draw_async(self->lcd, req);
	}
}

void hori_bar_show(struct HoriBar* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Dialog;
		req->x1 = HORI_BAR_X1;
		req->y1 = HORI_BAR_Y1;
		req->x2 = HORI_BAR_X2;
		req->y2 = HORI_BAR_Y2;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		lcd_draw_async(self->lcd, req);
	}

	_hori_bar_show_bar(self);
}

void hori_bar_hide(struct HoriBar* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Clear;
		req->x1 = HORI_BAR_X1;
		req->y1 = HORI_BAR_Y1;
		req->x2 = HORI_BAR_X2;
		req->y2 = HORI_BAR_Y2;

		lcd_draw_async(self->lcd, req);
	}
}

void hori_bar_set_state(struct HoriBar* self, u32 state) {
	if (state == self->state) return;

	self->state = state;

	_hori_bar_show_bar(self);
}
