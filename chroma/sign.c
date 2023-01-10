/*
 * vol.c
 *
 *  Created on: Oct 28, 2022
 *      Author: bhat
 */

#include "sign.h"

#include "../io/lcd.h"

#include <stdlib.h>

struct Sign sign_new() {
	struct Sign sign = {
		.lcd = 0,

		.octave = 0,
		.note = 0,
		.highlight = false,
	};

	return sign;
}

void sign_initialize(
	struct Sign* self,
	struct Lcd* lcd
) {
	self->lcd = lcd;
}

#define SIGN_X1 110
#define SIGN_Y1 10
#define SIGN_X2 230
#define SIGN_Y2 120
#define SIGN_MARGIN 16

void _sign_draw(struct Sign* self);
void _sign_draw_partial(struct Sign* self);

void sign_show(struct Sign* self) {
	_sign_draw(self);
}

void sign_hide(struct Sign* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Clear;
		req->x1 = SIGN_X1;
		req->y1 = SIGN_Y1;
		req->x2 = SIGN_X2;
		req->y2 = SIGN_Y2;

		lcd_draw_async(self->lcd, req);
	}
}

void sign_set_states(struct Sign* self, u32 octave, u32 note) {
	if (self->octave == octave && self->note == note) {
		return;
	}

	self->octave = octave;
	self->note = note;

	_sign_draw_partial(self);
}

static const char* const OCTAVE_STR_MAP[] = {
	"#", "#", "2", "3", "4", "5", "6", "7", "8"
};

static const char* const NOTE_STR_MAP[] = {
	"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

void _sign_draw_text(struct Sign* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Big_String;
		req->x1 = SIGN_X1 + SIGN_MARGIN;
		req->y1 = SIGN_Y1 + SIGN_MARGIN + 20;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = NOTE_STR_MAP[self->note];

		lcd_draw_async(self->lcd, req);
	}

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_String;
		req->x1 = SIGN_X1 + SIGN_MARGIN;
		req->y1 = SIGN_Y1 + SIGN_MARGIN;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = "Octave: ";

		lcd_draw_async(self->lcd, req);
	}

	if (self->highlight) {
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_String;
		req->x1 = SIGN_X1 + SIGN_MARGIN + 70;
		req->y1 = SIGN_Y1 + SIGN_MARGIN;

		req->pix.r = 0;
		req->pix.g = 0;
		req->pix.b = 0;
		req->pix_alt.r = 255;
		req->pix_alt.g = 255;
		req->pix_alt.b = 255;

		req->aux_ptr = OCTAVE_STR_MAP[self->octave];

		lcd_draw_async(self->lcd, req);
	} else {
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_String;
		req->x1 = SIGN_X1 + SIGN_MARGIN + 70;
		req->y1 = SIGN_Y1 + SIGN_MARGIN;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = OCTAVE_STR_MAP[self->octave];

		lcd_draw_async(self->lcd, req);
	}
}

void _sign_draw(struct Sign* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Dialog;
		req->x1 = SIGN_X1;
		req->y1 = SIGN_Y1;
		req->x2 = SIGN_X2;
		req->y2 = SIGN_Y2;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		lcd_draw_async(self->lcd, req);
	}

	if (self->highlight) {
		{
			struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
			req->type = lcd_request_Rect;
			req->x1 = SIGN_X1 + SIGN_MARGIN + 65;
			req->y1 = SIGN_Y1 + SIGN_MARGIN;
			req->x2 = SIGN_X2 - SIGN_MARGIN;
			req->y2 = SIGN_Y1 + SIGN_MARGIN + 25;

			req->pix.r = 255;
			req->pix.g = 255;
			req->pix.b = 255;

			lcd_draw_async(self->lcd, req);
		}
	}

	_sign_draw_text(self);
}

void _sign_draw_partial(struct Sign* self) {
	if (self->highlight) {
		{
			struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
			req->type = lcd_request_Rect;
			req->x1 = SIGN_X1 + SIGN_MARGIN + 65;
			req->y1 = SIGN_Y1 + SIGN_MARGIN;
			req->x2 = SIGN_X2 - SIGN_MARGIN;
			req->y2 = SIGN_Y1 + SIGN_MARGIN + 25;

			req->pix.r = 255;
			req->pix.g = 255;
			req->pix.b = 255;

			lcd_draw_async(self->lcd, req);
		}

		{
			struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
			req->type = lcd_request_String;
			req->x1 = SIGN_X1 + SIGN_MARGIN + 70;
			req->y1 = SIGN_Y1 + SIGN_MARGIN;

			req->pix.r = 0;
			req->pix.g = 0;
			req->pix.b = 0;
			req->pix_alt.r = 255;
			req->pix_alt.g = 255;
			req->pix_alt.b = 255;

			req->aux_ptr = OCTAVE_STR_MAP[self->octave];

			lcd_draw_async(self->lcd, req);
		}
	} else {
		{
			struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
			req->type = lcd_request_Rect;
			req->x1 = SIGN_X1 + SIGN_MARGIN + 65;
			req->y1 = SIGN_Y1 + SIGN_MARGIN;
			req->x2 = SIGN_X2 - SIGN_MARGIN;
			req->y2 = SIGN_Y1 + SIGN_MARGIN + 25;

			req->pix.r = 0;
			req->pix.g = 0;
			req->pix.b = 0;

			lcd_draw_async(self->lcd, req);
		}

		{
			struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
			req->type = lcd_request_String;
			req->x1 = SIGN_X1 + SIGN_MARGIN + 70;
			req->y1 = SIGN_Y1 + SIGN_MARGIN;

			req->pix.r = 255;
			req->pix.g = 255;
			req->pix.b = 255;
			req->pix_alt.r = 0;
			req->pix_alt.g = 0;
			req->pix_alt.b = 0;

			req->aux_ptr = OCTAVE_STR_MAP[self->octave];

			lcd_draw_async(self->lcd, req);
		}
	}

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Rect;
		req->x1 = SIGN_X1 + SIGN_MARGIN;
		req->y1 = SIGN_Y1 + SIGN_MARGIN + 30;
		req->x2 = SIGN_X1 + SIGN_MARGIN + 80;
		req->y2 = SIGN_Y2 - SIGN_MARGIN;

		req->pix.r = 0;
		req->pix.g = 0;
		req->pix.b = 0;

		lcd_draw_async(self->lcd, req);
	}

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Big_String;
		req->x1 = SIGN_X1 + SIGN_MARGIN;
		req->y1 = SIGN_Y1 + SIGN_MARGIN + 20;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = NOTE_STR_MAP[self->note];

		lcd_draw_async(self->lcd, req);
	}
}
