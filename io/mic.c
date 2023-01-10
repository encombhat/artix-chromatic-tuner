/*
 * mic.c
 *
 *  Created on: Nov 12, 2022
 *      Author: bhat
 */

#include "mic.h"

#include <string.h>

struct Mic mic_new(u32 base_addr) {
	struct Mic mic = {
		.event_loop = 0,
		.ts_now_ptr = 0,
		.base_addr = base_addr,

		.buf = {0},

		.merge_count = 0,
		.batch_number = 0,
		.fill_buf = 0,

		.on_check = signal_new(),
		.on_mate = signal_new(),
	};

	return mic;
}

void mic_initialize(
	struct Mic* self,
	struct EventLoop* event_loop,
	u32* ts_now_ptr
) {
	self->event_loop = event_loop;
	self->ts_now_ptr = ts_now_ptr;

	volatile u32* _base_addr_ptr = self->base_addr;
	*_base_addr_ptr = 0;
}

#define MIC_SAMPLES_CAPTURED_OFFSET 0
#define MIC_READOUT_ADDR_OFFSET 4
#define MIC_READOUT_VALUE_OFFSET 8

void _mic_fill_blocking_full(
	struct Mic* self,
	short* buf,
	u32 merge_count
) {
	u32 batch_size = (MIC_SAMPLE_SIZE * merge_count) / MIC_RAW_SAMPLE_SIZE;
//	xil_printf("batch size: %d\r\n", batch_size);

	volatile u32* _samples_captured_ptr = self->base_addr + MIC_SAMPLES_CAPTURED_OFFSET;
	volatile u32* _readout_addr_ptr = self->base_addr + MIC_READOUT_ADDR_OFFSET;
	volatile int* _readout_value_ptr = self->base_addr + MIC_READOUT_VALUE_OFFSET;
	volatile u32* _base_addr_ptr = self->base_addr;

	for (u32 b = 0; b < batch_size; b += 1) {
		while ((*_samples_captured_ptr) < MIC_RAW_SAMPLE_SIZE) {}
		*_base_addr_ptr = 0;

		for (u32 i = 0; i < MIC_RAW_SAMPLE_SIZE / merge_count; i += 1) {
			long int _val = 0;

			for (u32 k = 0; k < merge_count; k += 1) {
				u32 idx = i * merge_count + k;

				*_readout_addr_ptr = idx;
				_val += *_readout_value_ptr;
			}

			int val = _val / merge_count;
//			xil_printf("val: %d\r\n", val);

			// 3.3 * 4096 / 67108864, what does this mean?
			float factor = 0.0002;
			buf[b * MIC_RAW_SAMPLE_SIZE / merge_count + i] = (short) (factor * val);
		}
	}
}

void _mic_fill_blocking_nonfull(
	struct Mic* self,
	short* buf,
	u32 merge_count
) {
	u32 one_batch = merge_count * 1024;
//	xil_printf("batch size: %d\r\n", batch_size);

	volatile u32* _samples_captured_ptr = self->base_addr + MIC_SAMPLES_CAPTURED_OFFSET;
	volatile u32* _readout_addr_ptr = self->base_addr + MIC_READOUT_ADDR_OFFSET;
	volatile int* _readout_value_ptr = self->base_addr + MIC_READOUT_VALUE_OFFSET;
	volatile u32* _base_addr_ptr = self->base_addr;

	while ((*_samples_captured_ptr) < one_batch) {}
	*_base_addr_ptr = 0;

	for (u32 i = 0; i < 1024; i += 1) {
		long int _val = 0;

		for (u32 k = 0; k < merge_count; k += 1) {
			u32 idx = i * merge_count + k;

			*_readout_addr_ptr = idx;
			_val += *_readout_value_ptr;
		}

		int val = _val / merge_count;
//			xil_printf("val: %d\r\n", val);

		// 3.3 * 4096 / 67108864, what does this mean?
		float factor = 0.0002;
		buf[i] = (short) (factor * val);
	}
}

void mic_fill_blocking(
	struct Mic* self,
	short* buf,
	u32 merge_count
) {
	memset(buf, 0, sizeof(short) * MIC_SAMPLE_SIZE);

	if (merge_count < 4) {
		return _mic_fill_blocking_nonfull(self, buf, merge_count);
	} else {
		return _mic_fill_blocking_full(self, buf, merge_count);
	}
}

void mic_fill_async(
	struct Mic* self,
	short* buf,
	u32 merge_count // 4, 8, 16, etc.
) {
	if (self->fill_buf) return;

	if (merge_count < 4) { // Fill the buffer immediately
		_mic_fill_blocking_nonfull(self, buf, merge_count);

		signal_trigger(
			&(self->on_mate),
			self->event_loop, (void*) merge_count
		);
		return;
	}

	self->merge_count = merge_count;
	self->batch_number = 0;
	self->fill_buf = buf;
}

void _mic_cap(struct Mic* self) {
	volatile u32* _samples_captured_ptr = self->base_addr + MIC_SAMPLES_CAPTURED_OFFSET;
	volatile u32* _readout_addr_ptr = self->base_addr + MIC_READOUT_ADDR_OFFSET;
	volatile int* _readout_value_ptr = self->base_addr + MIC_READOUT_VALUE_OFFSET;
	volatile u32* _base_addr_ptr = self->base_addr;

	if ((*_samples_captured_ptr) < MIC_RAW_SAMPLE_SIZE) return;
	*_base_addr_ptr = 0;

	u32 merge_count = 4;
	for (u32 i = 0; i < MIC_RAW_SAMPLE_SIZE / merge_count; i += 1) {
		long int _val = 0;

		for (u32 k = 0; k < merge_count; k += 1) {
			u32 idx = i * merge_count + k;

			*_readout_addr_ptr = idx;
			_val += *_readout_value_ptr;
		}

		int val = _val / merge_count;
//			xil_printf("val: %d\r\n", val);

		// 3.3 * 4096 / 67108864, what does this mean?
		float factor = 0.0002;
		self->buf[i] = (short) (factor * val);
	}

	signal_trigger(
		&(self->on_check),
		self->event_loop, (void*) self->buf
	);
}

void _mic_cap_fill(
	struct Mic* self
) {
	u32 batch_size = (MIC_SAMPLE_SIZE * self->merge_count) / MIC_RAW_SAMPLE_SIZE;
//	xil_printf("batch size: %d\r\n", batch_size);

	volatile u32* _samples_captured_ptr = self->base_addr + MIC_SAMPLES_CAPTURED_OFFSET;
	volatile u32* _readout_addr_ptr = self->base_addr + MIC_READOUT_ADDR_OFFSET;
	volatile int* _readout_value_ptr = self->base_addr + MIC_READOUT_VALUE_OFFSET;
	volatile u32* _base_addr_ptr = self->base_addr;

	if ((*_samples_captured_ptr) < MIC_RAW_SAMPLE_SIZE - 1024) return;

//	xil_printf("mic:filling requested buf: batch %d\r\n", self->batch_number);

	while ((*_samples_captured_ptr) < MIC_RAW_SAMPLE_SIZE) {} // yea
	*_base_addr_ptr = 0;

	for (u32 i = 0; i < MIC_RAW_SAMPLE_SIZE / self->merge_count; i += 1) {
		long int _val = 0;

		for (u32 k = 0; k < self->merge_count; k += 1) {
			u32 idx = i * self->merge_count + k;

			*_readout_addr_ptr = idx;
			_val += *_readout_value_ptr;
		}

		int val = _val / self->merge_count;
//		xil_printf("val: %d\r\n", val);

		// 3.3 * 4096 / 67108864, what does this mean?
		float factor = 0.0002;
		self->fill_buf[self->batch_number * MIC_RAW_SAMPLE_SIZE / self->merge_count + i] = (short) (factor * val);
	}

	self->batch_number += 1;
	if (self->batch_number == batch_size) {
		signal_trigger(
			&(self->on_mate),
			self->event_loop, (void*) self->merge_count
		);

		self->merge_count = 0;
		self->batch_number = 0;
		self->fill_buf = 0;
	}
}

void mic_idle(struct Mic* self) {
	if (!self->fill_buf) {
		_mic_cap(self);
	} else {
		_mic_cap_fill(self);
	}
}
