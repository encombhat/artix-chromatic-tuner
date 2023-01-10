/*
 * buf.h
 *
 *  Created on: Oct 20, 2022
 *      Author: bhat
 */

#ifndef SRC_IO_BUF_H_
#define SRC_IO_BUF_H_

#include <stdbool.h>

#include <xil_types.h>

struct BufAux {
	u32 size;

	u32 head;
	u32 tail;
};

void bufaux_push(struct BufAux* self);
void bufaux_pop(struct BufAux* self);

bool buf_empty(struct BufAux aux);
bool buf_full(struct BufAux aux);

void buf_push(u32* data, u32 item, struct BufAux aux);
u32 buf_pop(u32* data, struct BufAux aux);

#endif /* SRC_IO_BUF_H_ */
