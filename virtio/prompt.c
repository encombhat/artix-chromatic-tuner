/*
 * prompt.c
 *
 *  Created on: Oct 29, 2022
 *      Author: bhat
 */

#include "prompt.h"

#include "../io/lcd.h"

#include <stdlib.h>

struct Prompt prompt_new(
	const char* header_str,
	const char* str,
	u32 header_width,
	u32 dialog_height
) {
	struct Prompt prompt = {
		.lcd = 0,

		.header_width = header_width,
		.dialog_height = dialog_height,
		.header_str = header_str,
		.str = str,
	};

	return prompt;
}

struct CompatPrompt compat_prompt_new(
	const char* str,
	u32 dialog_height
) {
	struct CompatPrompt prompt = {
		.lcd = 0,

		.dialog_height = dialog_height,
		.str = str,
	};

	return prompt;
}

void prompt_initialize(
	struct Prompt* self,
	struct Lcd* lcd
) {
	self->lcd = lcd;
}

void compat_prompt_initialize(
	struct CompatPrompt* self,
	struct Lcd* lcd
) {
	self->lcd = lcd;
}

#define PROMPT_HEADER_X1 10
#define PROMPT_HEADER_X2 75
#define PROMPT_HEADER_HEIGHT 40

#define PROMPT_HEADER_HORI_MARGIN 10
#define PROMPT_HEADER_VERT_MARGIN 8
#define PROMPT_PADDING 5

#define PROMPT_X1 10
#define PROMPT_X2 230
#define PROMPT_Y2 310

#define PROMPT_HORI_MARGIN 16
#define PROMPT_VERT_MARGIN 10

void prompt_show(struct Prompt* self) {
	u32 prompt_y1 = PROMPT_Y2 - self->dialog_height;
	u32 prompt_header_x2 = PROMPT_HEADER_X1 + self->header_width;
	u32 prompt_header_y2 = prompt_y1 - PROMPT_PADDING;
	u32 prompt_header_y1 = prompt_header_y2 - PROMPT_HEADER_HEIGHT;

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Dialog;
		req->x1 = PROMPT_HEADER_X1;
		req->y1 = prompt_header_y1;
		req->x2 = prompt_header_x2;
		req->y2 = prompt_header_y2;

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
		req->x1 = PROMPT_HEADER_X1 + PROMPT_HEADER_HORI_MARGIN;
		req->y1 = prompt_header_y1 + PROMPT_HEADER_VERT_MARGIN;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = self->header_str;

		lcd_draw_async(self->lcd, req);
	}

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Dialog;
		req->x1 = PROMPT_X1;
		req->y1 = prompt_y1;
		req->x2 = PROMPT_X2;
		req->y2 = PROMPT_Y2;

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
		req->x1 = PROMPT_X1 + PROMPT_HORI_MARGIN;
		req->y1 = prompt_y1 + PROMPT_VERT_MARGIN;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = self->str;

		lcd_draw_async(self->lcd, req);
	}
}

void compat_prompt_show(struct CompatPrompt* self) {
	u32 prompt_y1 = PROMPT_Y2 - self->dialog_height;

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Dialog;
		req->x1 = PROMPT_X1;
		req->y1 = prompt_y1;
		req->x2 = PROMPT_X2;
		req->y2 = PROMPT_Y2;

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
		req->x1 = PROMPT_X1 + PROMPT_HORI_MARGIN;
		req->y1 = prompt_y1 + PROMPT_VERT_MARGIN;

		req->pix.r = 255;
		req->pix.g = 255;
		req->pix.b = 255;
		req->pix_alt.r = 0;
		req->pix_alt.g = 0;
		req->pix_alt.b = 0;

		req->aux_ptr = self->str;

		lcd_draw_async(self->lcd, req);
	}
}

void prompt_hide(struct Prompt* self) {
	u32 prompt_y1 = PROMPT_Y2 - self->dialog_height;
	u32 prompt_header_x2 = PROMPT_HEADER_X1 + self->header_width;
	u32 prompt_header_y2 = prompt_y1 - PROMPT_PADDING;
	u32 prompt_header_y1 = prompt_header_y2 - PROMPT_HEADER_HEIGHT;

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Clear;
		req->x1 = PROMPT_HEADER_X1;
		req->y1 = prompt_header_y1;
		req->x2 = prompt_header_x2;
		req->y2 = prompt_header_y2;

		lcd_draw_async(self->lcd, req);
	}

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Clear;
		req->x1 = PROMPT_X1;
		req->y1 = prompt_y1;
		req->x2 = PROMPT_X2;
		req->y2 = PROMPT_Y2;

		lcd_draw_async(self->lcd, req);
	}
}

void compat_prompt_hide(struct CompatPrompt* self) {
	u32 prompt_y1 = PROMPT_Y2 - self->dialog_height;

	{
		struct DrawRequest* req = malloc(sizeof(struct DrawRequest));
		req->type = lcd_request_Clear;
		req->x1 = PROMPT_X1;
		req->y1 = prompt_y1;
		req->x2 = PROMPT_X2;
		req->y2 = PROMPT_Y2;

		lcd_draw_async(self->lcd, req);
	}
}
