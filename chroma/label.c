/*
 * label.c
 *
 *  Created on: Dec 2, 2022
 *      Author: bhat
 */

#include "label.h"
#include <stdio.h>

struct ChromaLabel chroma_label_new(
	struct Lcd* lcd,
	u32 x1, u32 y1, u32 x2, u32 y2,
	char* pre_text, u32 pre_margin,
	char* post_text, u32 post_margin
) {
	struct ChromaLabel label = {
		.lcd = lcd,

		.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2,

		.pre_text = pre_text,
		.pre_margin = pre_margin,
		.state = 0,
		.post_text = post_text,
		.post_margin = post_margin,
	};
	return label;
}

void chroma_label_initialize(struct ChromaLabel* self) {}

#define CHROMA_LABEL_MARGIN 10

void _chroma_label_text(struct ChromaLabel* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_String;
		req->x1 = self->x1 + CHROMA_LABEL_MARGIN + self->pre_margin;
		req->y1 = self->y1 + CHROMA_LABEL_MARGIN;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = self->buf;

		lcd_draw_async(self->lcd, req);
	}
}

void _chroma_label_draw_partial(struct ChromaLabel* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Rect;
		req->x1 = self->x1 + CHROMA_LABEL_MARGIN + self->pre_margin;
		req->y1 = self->y1 + CHROMA_LABEL_MARGIN;
		req->x2 = self->x2 - CHROMA_LABEL_MARGIN - self->post_margin;
		req->y2 = self->y2 - CHROMA_LABEL_MARGIN;

		req->pix.r = 0;
		req->pix.g = 0;
		req->pix.b = 0;

		lcd_draw_async(self->lcd, req);
	}

	_chroma_label_text(self);
}

void chroma_label_show(struct ChromaLabel* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Dialog;
		req->x1 = self->x1;
		req->y1 = self->y1;
		req->x2 = self->x2;
		req->y2 = self->y2;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		lcd_draw_async(self->lcd, req);
	}

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_String;
		req->x1 = self->x1 + CHROMA_LABEL_MARGIN;
		req->y1 = self->y1 + CHROMA_LABEL_MARGIN;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = self->pre_text;

		lcd_draw_async(self->lcd, req);
	}

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_String;
		req->x1 = self->x2 - CHROMA_LABEL_MARGIN - self->post_margin;
		req->y1 = self->y1 + CHROMA_LABEL_MARGIN;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = self->post_text;

		lcd_draw_async(self->lcd, req);
	}

	_chroma_label_text(self);
}

void chroma_label_hide(struct ChromaLabel* self) {
	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Clear;
		req->x1 = self->x1;
		req->y1 = self->y1;
		req->x2 = self->x2;
		req->y2 = self->y2;

		lcd_draw_async(self->lcd, req);
	}
}

void chroma_label_refresh(struct ChromaLabel* self) {
	_chroma_label_draw_partial(self);
}

void chroma_label_set_state(struct ChromaLabel* self, u32 state) {
	if (self->state == state) return;

	self->state = state;
	sprintf(self->buf, "%d", (int) state);
}
