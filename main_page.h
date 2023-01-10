/*
 * main_page.h
 *
 *  Created on: Dec 1, 2022
 *      Author: bhat
 */

#ifndef SRC_MAIN_PAGE_H_
#define SRC_MAIN_PAGE_H_

#include "app.h"

struct MainPage;
void main_page_initialize(struct MainPage* self);
void main_page_destroy(struct MainPage* self);

struct MainPage {
	struct App* app;

	struct CompatPrompt prompt;
};
struct MainPage* main_page_new(struct App* app);

#endif /* SRC_MAIN_PAGE_H_ */
