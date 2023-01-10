/*
 * hist.h
 *
 *  Created on: Dec 2, 2022
 *      Author: bhat
 */

#ifndef SRC_HIST_H_
#define SRC_HIST_H_

#include "io/lcd.h"
#include "chroma/label.h"

#include "app.h"

struct Hist;
void hist_initialize(struct Hist* self);
void hist_destroy(struct Hist* self);
void hist_tick(
	struct Hist* self,
	u32 ts_now
);

struct Hist {
	struct App* app;

	struct VirtTone tone;

	struct VirtPeriodic periodic;

	struct ChromaLabel freq_label;
	struct ChromaLabel spacing_label;
};
struct Hist* hist_new(struct App* app);

#endif /* SRC_HIST_H_ */
