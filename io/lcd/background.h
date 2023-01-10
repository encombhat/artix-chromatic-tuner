/*
 * lcd_background.h
 *
 *  Created on: Oct 29, 2022
 *      Author: bhat
 */

#ifndef SRC_IO_LCD_BACKGROUND_H_
#define SRC_IO_LCD_BACKGROUND_H_

#include "xil_types.h"

struct GIMPImage {
  u32  	 width;
  u32  	 height;
  u32  	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  u8 	 pixel_data[240 * 320 * 4 + 1];
};

extern struct GIMPImage lcd_background;

#endif /* SRC_IO_LCD_BACKGROUND_H_ */
