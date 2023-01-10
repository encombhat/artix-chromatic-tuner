/*
 * vol.h
 *
 *  Created on: Oct 28, 2022
 *      Author: bhat
 */

#ifndef SRC_VIRTIO_VOL_H_
#define SRC_VIRTIO_VOL_H_

#include "../io/lcd.h"

#include <stdbool.h>

struct Sign;
void sign_initialize(
	struct Sign* self,
	struct Lcd* lcd
);
void sign_set_states(struct Sign* self, u32 octave, u32 note);
void sign_show(struct Sign* self);
void sign_hide(struct Sign* self);

struct Sign {
	struct Lcd* lcd;

	u32 octave;
	u32 note;
	bool highlight;

	// Signals
};

struct Sign sign_new();

#endif /* SRC_VIRTIO_VOL_H_ */
