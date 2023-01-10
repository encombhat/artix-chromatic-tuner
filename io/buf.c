/*
 * buf.c
 *
 *  Created on: Oct 20, 2022
 *      Author: bhat
 */

#include "buf.h"

static inline u32 inc(u32 cur, u32 size) {
	return (cur + 1) % size;
}

void bufaux_push(struct BufAux* self) {
	if (buf_full(*self)) {
		self->head = inc(self->head, self->size);
	}
	self->tail = inc(self->tail, self->size);
}

void bufaux_pop(struct BufAux* self) {
	self->head = inc(self->head, self->size);
}

bool buf_empty(struct BufAux aux) {
	return aux.head == aux.tail;
}

bool buf_full(struct BufAux aux) {
	return aux.head == (aux.tail + 1) % aux.size;
}

void buf_push(u32* data, u32 item, struct BufAux aux) {
	data[aux.tail] = item;
}

u32 buf_pop(u32* data, struct BufAux aux) {
	if (buf_empty(aux)) return 0;

	return data[aux.head];
}
