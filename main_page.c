/*
 * main_page.c
 *
 *  Created on: Dec 1, 2022
 *      Author: bhat
 */

#include "main_page.h"

struct MainPage* main_page_new(struct App* app) {
	struct MainPage* main_page = malloc(sizeof(struct MainPage));
	main_page->app = app;

	main_page->prompt = compat_prompt_new(
		"Left: Chromatic Tuner\nRight: Histogram",
		70
	);
	return main_page;
}

void main_page_initialize(struct MainPage* self) {
	compat_prompt_initialize(&(self->prompt), &(self->app->lcd));

	compat_prompt_show(&(self->prompt));
}

void main_page_destroy(struct MainPage* self) {
	compat_prompt_hide(&(self->prompt));
}
