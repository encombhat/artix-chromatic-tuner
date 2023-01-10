/*
 * prompt.h
 *
 *  Created on: Oct 29, 2022
 *      Author: bhat
 */

#ifndef SRC_VIRTIO_PROMPT_H_
#define SRC_VIRTIO_PROMPT_H_

#include "../event.h"
#include "../io/lcd.h"

struct Prompt;
void prompt_initialize(
	struct Prompt* self,
	struct Lcd* lcd
);
void prompt_show(struct Prompt* self);
void prompt_hide(struct Prompt* self);

struct Prompt {
	struct Lcd* lcd;

	u32 header_width;
	u32 dialog_height;
	const char* header_str;
	const char* str;

	// Signals
};
struct Prompt prompt_new(
	const char* header_str,
	const char* str,
	u32 header_width,
	u32 dialog_height
);

struct CompatPrompt;
void compat_prompt_initialize(
	struct CompatPrompt* self,
	struct Lcd* lcd
);
void compat_prompt_show(struct CompatPrompt* self);
void compat_prompt_hide(struct CompatPrompt* self);

struct CompatPrompt {
	struct Lcd* lcd;

	u32 dialog_height;
	const char* str;

	// Signals
};
struct CompatPrompt compat_prompt_new(
	const char* str,
	u32 dialog_height
);

#endif /* SRC_VIRTIO_PROMPT_H_ */
