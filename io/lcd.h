/*
 * lcd.h
 *
 *  Created on: Oct 27, 2022
 *      Author: bhat
 */

#ifndef SRC_IO_LCD_H_
#define SRC_IO_LCD_H_

#include "xgpio.h"
#include "xspi.h"
#include "xil_types.h"

#include "../event.h"

struct Pixel {
	u8 r;
	u8 g;
	u8 b;
};

enum DrawRequestType {
	lcd_request_None = 0,

	lcd_request_Rect = 10,
	lcd_request_Dialog,
	lcd_request_String,
	lcd_request_Big_String,
	lcd_request_Histogram,
	lcd_request_Clear,

	lcd_request_ClearAll,

	lcd_request_Unknown = 100,
};

struct DrawRequest {
	enum DrawRequestType type;

	u32 x1; u32 y1;
	u32 x2; u32 y2;

	struct Pixel pix;
	struct Pixel pix_alt;

	void* aux_ptr;
};

struct Lcd;
void lcd_initialize(
	struct Lcd* self,
	struct EventLoop* event_loop
);
void lcd_draw_async(struct Lcd* self, struct DrawRequest* req);

// TODO: Replace lcd_driv, that code is garbage
struct Lcd {
	struct EventLoop* event_loop;

	u8 gpio_device_id;
	u8 spi_device_id;

	struct Pixel default_pix;

	XGpio sys_gpio;
	XSpi sys_spi;
};
struct Lcd lcd_new(
	u8 gpio_device_id,
	u8 spi_device_id
);

#endif /* SRC_IO_LCD_H_ */
