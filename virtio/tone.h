/*
 * tune.h
 *
 *  Created on: Nov 12, 2022
 *      Author: bhat
 */

#ifndef SRC_VIRTIO_TONE_H_
#define SRC_VIRTIO_TONE_H_

#include <xil_types.h>

struct VirtTone;
u32 virt_tone_identify(
	struct VirtTone* self,
	u32 A4,
	short* buf, short* buf_aux
); // buf must be VIRT_TONE_BUF_SIZE long
u32 virt_tone_identify_with_octave(
	struct VirtTone* self,
	u32 A4, u32 octave,
	short* buf, short* buf_aux
); // buf must be VIRT_TONE_BUF_SIZE long
u32 virt_tone_max_freq(
	struct VirtTone* self,
	short* buf, short* buf_aux
); // buf must be VIRT_TONE_BUF_SIZE long

// Same as MIC_SAMPLE_SIZE
#define VIRT_TONE_BUF_SIZE 1024
#define VIRT_TONE_N 10

struct VirtTone {
	float f_s;
	u32 min_amplitude;
};
struct VirtTone virt_tone_new(float f_s, u32 min_amplitude);

#endif /* SRC_VIRTIO_TONE_H_ */
