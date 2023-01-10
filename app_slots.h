/*
 * app_slots.h
 *
 *  Created on: Oct 22, 2022
 *      Author: bhat
 */

#ifndef SRC_APP_SLOTS_H_
#define SRC_APP_SLOTS_H_

#include <stdbool.h>

#include "xil_types.h"

struct App;

void _app_connect_signal_slot(struct App* self);

void _app_main_page_triggered_slot(struct App* self, void* aux_data);

#endif /* SRC_APP_SLOTS_H_ */
