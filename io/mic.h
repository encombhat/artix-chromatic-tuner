/*
 * mic.h
 *
 *  Created on: Nov 12, 2022
 *      Author: bhat
 */

#ifndef SRC_IO_MIC_H_
#define SRC_IO_MIC_H_

#include "../event.h"

#include <xil_types.h>

struct Mic;
void mic_initialize(
	struct Mic* self,
	struct EventLoop* event_loop,
	u32* ts_now_ptr
);
// Assumes buf with length 1024
void mic_fill_blocking(
	struct Mic* self,
	short* buf,
	u32 merge_count // 4, 8, 16, etc.
);
void mic_fill_async(
	struct Mic* self,
	short* buf,
	u32 merge_count // 4, 8, 16, etc.
);

void mic_idle(struct Mic* self);

#define MIC_SAMPLE_SIZE 1024
#define MIC_RAW_SAMPLE_SIZE 4096

struct Mic {
	struct EventLoop* event_loop;
	u32* ts_now_ptr;

	u32 base_addr;

	short buf[MIC_SAMPLE_SIZE];

	u32 merge_count;
	u32 batch_number;
	short* fill_buf;

	// Signals
	struct Signal on_check;
	struct Signal on_mate;
};
struct Mic mic_new(u32 base_addr);

#endif /* SRC_IO_MIC_H_ */
