/*
 * label.h
 *
 *  Created on: Dec 2, 2022
 *      Author: bhat
 */

#ifndef SRC_CHROMA_LABEL_H_
#define SRC_CHROMA_LABEL_H_

#include "../io/lcd.h"

struct ChromaLabel;
void chroma_label_initialize(struct ChromaLabel* self);
void chroma_label_show(struct ChromaLabel* self);
void chroma_label_hide(struct ChromaLabel* self);
void chroma_label_refresh(struct ChromaLabel* self);
void chroma_label_set_state(struct ChromaLabel* self, u32 state);

struct ChromaLabel {
	struct Lcd* lcd;

	u32 x1, y1, x2, y2;

	char* pre_text;
	u32 pre_margin;
	u32 state;
	char* post_text;
	u32 post_margin;

	char buf[16];
};
struct ChromaLabel chroma_label_new(
	struct Lcd* lcd,
	u32 x1, u32 y1, u32 x2, u32 y2,
	char* pre_text, u32 pre_margin,
	char* post_text, u32 post_margin
);

#endif /* SRC_CHROMA_LABEL_H_ */
