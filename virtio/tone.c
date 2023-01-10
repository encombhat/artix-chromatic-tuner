/*
 * tone.c
 *
 *  Created on: Nov 12, 2022
 *      Author: bhat
 */

#include "tone.h"

#include "tone_impl/fixed_fft.h"

#include <string.h>
#include <stdbool.h>

struct VirtTone virt_tone_new(float f_s, u32 min_amplitude) {
	struct VirtTone tone = {
		.f_s = f_s,
		.min_amplitude = min_amplitude,
	};

	return tone;
}

static inline short _virt_tone_abs(short val) {
	if (val < 0) return -val;
	return val;
}

//double _virt_tone_freq(u32 A4, int octave, int tone) {
//	double freq = A4;
//	double exp_pre = tone;
//	exp_pre -= 9;
//	exp_pre /= 12.0;
//	exp_pre += octave;
//	exp_pre -= 4;
//	freq *= pow(2.0, exp_pre);
//	return freq;
//}

//u32 _virt_tone_deviation(
//	u32 A4, double freq, int octave, int tone
//) {
//	double freq_tone = _virt_tone_freq(A4, octave, tone);
//
//	u32 deviation;
//	if (freq < freq_tone) {
//		double freq_pre = _virt_tone_freq(A4, octave, tone - 1);
//		double diff = freq_tone - freq;
//		diff /= (freq_tone - freq_pre);
//		diff *= 100;
//		if (diff > 0 && diff < 50) {
//			deviation = 50 - diff;
//		} else {
//			deviation = 0;
//		}
//	} else {
//		double freq_aft = _virt_tone_freq(A4, octave, tone + 1);
//		double diff = freq - freq_tone;
//		diff /= (freq_aft - freq_tone);
//		diff *= 100;
//		if (diff > 0 && diff < 50) {
//			deviation = 50 + diff;
//		} else {
//			deviation = 100;
//		}
//	}
//
//	return deviation;
//}

u32 _virt_tone_deviation(
	u32 tone, double tone_fp
) {
//	xil_printf("tone: %d, fp: %d\r\n", tone, (u32) (tone_fp * 100.0));

	u32 deviation;
	if (tone_fp < tone) {
		double diff = tone;
		diff -= tone_fp;
		diff *= 100;
		if (diff > 0 && diff < 50) {
			deviation = 50 - diff;
		} else {
			deviation = 0;
		}
	} else {
		double diff = tone_fp;
		diff -= tone;
		diff *= 100;
		if (diff > 0 && diff < 50) {
			deviation = 50 + diff;
		} else {
			deviation = 100;
		}
	}

	return deviation;
}

#define TONE_PRE 3
#define TONE_POST 600

// freq << 16 | dev << 8 | octave << 4 | tone
u32 virt_tone_identify(
	struct VirtTone* self,
	u32 A4,
	short* buf, short* buf_aux
) {
	memset(buf_aux, 0, sizeof(short) * VIRT_TONE_BUF_SIZE);

	fix_fft(buf, buf_aux, VIRT_TONE_N);

	for (u32 i = TONE_PRE; i < TONE_POST; i += 1) { // TODO: Why??
		buf_aux[i] = _virt_tone_abs(buf[i]);
	}

	u32 idx_max = 2;
	int val_max = 0;
	for (u32 i = TONE_PRE; i < TONE_POST; i += 1) {
		if (buf_aux[i] > val_max) {
			idx_max = i;
			val_max = buf_aux[i];
		}
	}
	if (val_max < self->min_amplitude) {
		return 0;
	};

	int val_pre = buf_aux[idx_max - 1];
	int val_aft = buf_aux[idx_max + 1];

	float bin_spacing = self->f_s / VIRT_TONE_BUF_SIZE;
	float freq = idx_max * bin_spacing;
//	float diff = val_aft - val_pre;
//	diff /= val_max;
//	freq += diff * bin_spacing * 0.5;

	float x0 = (2 * bin_spacing * (val_max-val_pre)) / (2 * val_max-val_pre-val_aft);
	x0 = x0 / bin_spacing;

	if(x0 < 0 || x0 > 2) { //error
		return 0;
	}
	if(x0 <= 1)  {
		freq = freq - (1-x0) * bin_spacing;
	}
	else {
		freq = freq + (x0-1) * bin_spacing;
	}

	double variable = 2 + 9.0 / 12.0;
	variable = pow(2.0, variable); // TODO
	variable *= freq;
	variable /= A4;
	variable = log2(variable);
	u32 octave = floor(variable) + 2;

	if (octave < 2) { //error
		return 0;
	}
	if (octave > 8) { //error
		return 0;
	}

	double tone_fp = 12.0 * (variable - floor(variable));
	u32 tone = tone_fp + 0.5;

	u32 deviation = _virt_tone_deviation(tone, tone_fp);

	if (tone > 11) { // Huh
		tone_fp -= 12.0;
		tone = 0;
		octave += 1;
	}

	u32 result = (u32) freq;
	result <<= 16;
	result |= deviation << 8;
	result |= octave << 4;
	result |= tone;

	return result;
}

u32 _virt_tone_octave_range(struct VirtTone* self, u32 A4, u32 set_octave) {
	float bin_spacing = self->f_s / VIRT_TONE_BUF_SIZE;

	float freq_pre = A4;
	float freq_aft = A4;
	float exp = -9.f;
	exp /= 12.f;
	exp -= 4.f;
	exp += set_octave;
	freq_pre *= pow(2.0, exp);
	freq_aft *= pow(2.0, exp + 1);

	int pre_idx = freq_pre / bin_spacing - 1;
	if (pre_idx < 0) pre_idx = 0;
	int post_idx = freq_aft / bin_spacing + 1;
	if (post_idx > 1023) post_idx = 1023;

	return ((u32) pre_idx) << 16 | ((u32) post_idx);
}

u32 virt_tone_identify_with_octave(
	struct VirtTone* self,
	u32 A4, u32 set_octave,
	short* buf, short* buf_aux
) {
	memset(buf_aux, 0, sizeof(short) * VIRT_TONE_BUF_SIZE);

	fix_fft(buf, buf_aux, VIRT_TONE_N);

	{
		u32 octave_range = _virt_tone_octave_range(self, A4, set_octave);
		u32 octave_pre_idx = octave_range >> 16;
		u32 octave_aft_idx = octave_range & 0xFFFF;

//		xil_printf("octave pre: %d, aft: %d\r\n", octave_pre_idx, octave_aft_idx);

		for (int i = octave_pre_idx; i < octave_aft_idx; i += 1) {
			buf_aux[i] = _virt_tone_abs(buf[i]);
		}
	}

	u32 idx_max = 2;
	int val_max = 0;
	for (u32 i = TONE_PRE; i < TONE_POST; i += 1) {
		if (buf_aux[i] > val_max) {
			idx_max = i;
			val_max = buf_aux[i];
		}
	}
	if (val_max < self->min_amplitude) {
		return 0;
	};

	int val_pre = buf_aux[idx_max - 1];
	int val_aft = buf_aux[idx_max + 1];

	float bin_spacing = self->f_s / VIRT_TONE_BUF_SIZE;
	float freq = idx_max * bin_spacing;
//	float diff = val_aft - val_pre;
//	diff /= val_max;
//	freq += diff * bin_spacing * 0.5;

	float x0 = (2 * bin_spacing * (val_max-val_pre)) / (2 * val_max-val_pre-val_aft);
	x0 = x0 / bin_spacing;

	if(x0 < 0 || x0 > 2) { //error
		return 0;
	}
	if(x0 <= 1)  {
		freq = freq - (1-x0) * bin_spacing;
	}
	else {
		freq = freq + (x0-1) * bin_spacing;
	}

	double variable = 2 + 9.0 / 12.0;
	variable = pow(2.0, variable); // TODO
	variable *= freq;
	variable /= A4;
	variable = log2(variable);
	u32 octave = floor(variable) + 2;

	double tone_fp = 12.0 * (variable - floor(variable));
	u32 tone = tone_fp + 0.5;

	u32 deviation = _virt_tone_deviation(tone, tone_fp);

	if (tone > 11) { // Huh
		tone_fp -= 12.0;
		tone = 0;
		octave += 1;
	}
	if (octave < 2) { //error
		return 0;
	}
	if (octave > 8) { //error
		return 0;
	}
	if (octave != set_octave) {
		return 0;
	}

	u32 result = (u32) freq;
	result <<= 16;
	result |= deviation << 8;
	result |= octave << 4;
	result |= tone;

	return result;
}

u32 virt_tone_max_freq(
	struct VirtTone* self,
	short* buf, short* buf_aux
) {
	memset(buf_aux, 0, sizeof(short) * VIRT_TONE_BUF_SIZE);

	fix_fft(buf, buf_aux, VIRT_TONE_N);

	for (u32 i = TONE_PRE; i < TONE_POST; i += 1) {
		buf_aux[i] = _virt_tone_abs(buf[i]);
	}

	u32 idx_max = 2;
	int val_max = 0;
	for (
		u32 i = TONE_PRE;
		i < TONE_POST;
		i += 1
	) {
		if (buf_aux[i] > val_max) {
			idx_max = i;
			val_max = buf_aux[i];
		}
	}

	if (val_max < self->min_amplitude) {
		return 0;
	}

	float bin_spacing = self->f_s / VIRT_TONE_BUF_SIZE;
	float freq = idx_max * bin_spacing;

	return freq;
}
