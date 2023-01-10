/*
 * chroma.h
 *
 *  Created on: Dec 1, 2022
 *      Author: bhat
 */

#ifndef SRC_CHROMA_H_
#define SRC_CHROMA_H_

#include "event.h"
#include "app.h"

#include "virtio/oneshot.h"

#include "chroma/sign.h"
#include "chroma/hori_bar.h"
#include "chroma/label.h"

struct Chroma;
void chroma_initialize(
	struct Chroma* self
);
void chroma_destroy(
	struct Chroma* self
);
void chroma_tick(
	struct Chroma* self,
	u32 ts_now
);

enum ChromaState {
	Chroma_Nominal,
	Chroma_A4 = 10,
	Chroma_Octave,
};

struct Chroma {
	struct App* app;

	enum ChromaState state;
	u32 set_octave;

	struct VirtOneshot timer_encoder;

	struct Sign sign;
	struct HoriBar dev_bar;
	struct ChromaLabel label;
	struct ChromaLabel a4_label;

	struct VirtTone tone;
};
struct Chroma* chroma_new();

#endif /* SRC_CHROMA_H_ */
