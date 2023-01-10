/*
 * hori_bar.h
 *
 *  Created on: Dec 2, 2022
 *      Author: bhat
 */

#ifndef SRC_VIRTIO_HORI_BAR_H_
#define SRC_VIRTIO_HORI_BAR_H_

#include "../io/lcd.h"

struct HoriBar;
void hori_bar_initialize(
	struct HoriBar* self,
	struct Lcd* lcd
);
void hori_bar_show(struct HoriBar* self);
void hori_bar_hide(struct HoriBar* self);
void hori_bar_set_state(struct HoriBar* self, u32 state);

struct HoriBar {
	struct Lcd* lcd;

	u32 state;
};
struct HoriBar hori_bar_new();

#endif /* SRC_VIRTIO_HORI_BAR_H_ */
