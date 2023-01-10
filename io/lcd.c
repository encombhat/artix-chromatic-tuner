/*
 * lcd.c
 *
 *  Created on: Oct 27, 2022
 *      Author: bhat
 */

#include "lcd.h"
#include "xspi_l.h"

#include <stdlib.h>
#include "lcd/background.h"
#include "lcd/font.h"

void _lcd_init(struct Lcd* self);

struct Lcd lcd_new(
	u8 gpio_device_id,
	u8 spi_device_id
) {
	struct Lcd lcd = {
		.event_loop = 0,

		.gpio_device_id = gpio_device_id,
		.spi_device_id = spi_device_id,

		.default_pix = { .r = 100, .g = 100, .b = 100 },

		.sys_gpio = {},
		.sys_spi = {},
	};

	return lcd;
}

void lcd_initialize(
	struct Lcd* self,
	struct EventLoop* event_loop
) {
	self->event_loop = event_loop;

	{
		u32 status = XGpio_Initialize(&(self->sys_gpio), self->gpio_device_id);
		if (status != XST_SUCCESS)  {
			xil_printf("Initialize GPIO dc fail!\r\n");
			return;
		}
	}
	XGpio_SetDataDirection(&(self->sys_gpio), 1, 0x0);

	XSpi_Config* spiConfig = XSpi_LookupConfig(self->spi_device_id);
	if (spiConfig == NULL) {
		xil_printf("Can't find spi device!\r\n");
		return;
	}

	{
		u32 status = XSpi_CfgInitialize(&(self->sys_spi), spiConfig, spiConfig->BaseAddress);
		if (status != XST_SUCCESS) {
			xil_printf("Initialize spi fail!\r\n");
			return;
		}
	}

	XSpi_Reset(&(self->sys_spi));

	u32 controlReg = XSpi_GetControlReg(&(self->sys_spi));
	XSpi_SetControlReg(&(self->sys_spi),
			(controlReg | XSP_CR_ENABLE_MASK | XSP_CR_MASTER_MODE_MASK) &
			(~XSP_CR_TRANS_INHIBIT_MASK));

	// Select 1st slave device
	XSpi_SetSlaveSelectReg(&(self->sys_spi), ~0x01);

//	xil_printf("spi base addr: %d\r\n", self->sys_spi.BaseAddr);
//	xil_printf("gpio base addr: %d\r\n", self->sys_gpio.BaseAddress);

	_lcd_init(self);
}

 // The driver part
void _lcd_write_com(struct Lcd* self, char VL) {
    Xil_Out32(self->sys_gpio.BaseAddress, 0x0);
    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, VL);

    // Blocking until fifo is empty?
    while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
    Xil_Out32(
		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
	);
}

void _lcd_write_data(struct Lcd* self, char VL) {
    Xil_Out32(self->sys_gpio.BaseAddress, 0x01);
    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, VL);

    while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
    Xil_Out32(
		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
	);
}

void _lcd_write_data16(struct Lcd* self, char VH, char VL) {
    Xil_Out32(self->sys_gpio.BaseAddress, 0x01);
    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, VH);
    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, VL);

    while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
    Xil_Out32(
		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
	);
}

void _lcd_init(struct Lcd* self) {
    // Reset
    _lcd_write_com(self, 0x01);
    for (int i = 0; i < 500000; i += 1); //Must wait > 5ms

    _lcd_write_com(self, 0xCB);
    _lcd_write_data(self, 0x39);
    _lcd_write_data(self, 0x2C);
    _lcd_write_data(self, 0x00);
    _lcd_write_data(self, 0x34);
    _lcd_write_data(self,0x02);

    _lcd_write_com(self, 0xCF);
    _lcd_write_data(self, 0x00);
    _lcd_write_data(self, 0XC1);
    _lcd_write_data(self,0X30);

    _lcd_write_com(self, 0xE8);
    _lcd_write_data(self, 0x85);
    _lcd_write_data(self, 0x00);
    _lcd_write_data(self, 0x78);

    _lcd_write_com(self, 0xEA);
    _lcd_write_data(self, 0x00);
    _lcd_write_data(self, 0x00);

    _lcd_write_com(self, 0xED);
    _lcd_write_data(self, 0x64);
    _lcd_write_data(self, 0x03);
    _lcd_write_data(self, 0X12);
    _lcd_write_data(self, 0X81);

    _lcd_write_com(self, 0xF7);
    _lcd_write_data(self, 0x20);

    _lcd_write_com(self, 0xC0);   //Power control
    _lcd_write_data(self, 0x23);  //VRH[5:0]

    _lcd_write_com(self, 0xC1);   //Power control
    _lcd_write_data(self, 0x10);  //SAP[2:0];BT[3:0]

    _lcd_write_com(self, 0xC5);   //VCM control
    _lcd_write_data(self, 0x3e);  //Contrast
    _lcd_write_data(self, 0x28);

    _lcd_write_com(self, 0xC7);   //VCM control2
    _lcd_write_data(self, 0x86);  //--

    _lcd_write_com(self, 0x36);   // Memory Access Control
    _lcd_write_data(self, 0x48);

    _lcd_write_com(self, 0x3A);
    _lcd_write_data(self, 0x55);

    _lcd_write_com(self, 0xB1);
    _lcd_write_data(self, 0x00);
    _lcd_write_data(self, 0x18);

    _lcd_write_com(self, 0xB6);   // Display Function Control
    _lcd_write_data(self, 0x08);
    _lcd_write_data(self, 0x82);
    _lcd_write_data(self, 0x27);

    _lcd_write_com(self, 0x11);   //Exit Sleep
    for (int i = 0; i < 100000; i += 1); // What?

    _lcd_write_com(self, 0x29);   //Display on
    _lcd_write_com(self, 0x2c);
}

struct _LcdColorImpl {
	int byte_lower;
	int byte_higher;
};

struct _LcdColorImpl _lcd_color(struct Pixel* pix) {
	struct _LcdColorImpl impl = {
		.byte_lower = (pix->g & 0x1C) << 3 | pix->b >> 3,
		.byte_higher = (pix->r & 0x0F8) | pix->g >> 5,
	};

	return impl;
}

void _lcd_set_damaged(struct Lcd* self, int x1, int y1, int x2, int y2) {
	_lcd_write_com(self, 0x2A);
	_lcd_write_data(self, x1 >> 8);
	_lcd_write_data(self, x1);
	_lcd_write_data(self, x2 >> 8);
	_lcd_write_data(self, x2);
	_lcd_write_com(self, 0x2B);
    _lcd_write_data(self, y1 >> 8);
    _lcd_write_data(self, y1);
    _lcd_write_data(self, y2 >> 8);
    _lcd_write_data(self, y2);
    _lcd_write_com(self, 0x2C);
}

#define LCD_DRAW_BLOCK_SIZE 64

#define LCD_WIDTH 240
#define LCD_HEIGHT 320

void lcd_draw(struct Lcd* self, struct Pixel pix, int x1, int y1, int x2, int y2) {
    if (x1 > x2 || y1 > y2) return;

    _lcd_set_damaged(self, x1, y1, x2, y2);

    Xil_Out32(self->sys_gpio.BaseAddress, 0x01);

    struct _LcdColorImpl pix_impl = _lcd_color(&pix);

    int i = 0;

    for (int y = y1; y <= y2; y += 1) {
    	for (int x = x1; x <= x2; x += 1) {
            Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix_impl.byte_higher);
            Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix_impl.byte_lower);

            i += 1;

            if (i == LCD_DRAW_BLOCK_SIZE) {
                while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
                Xil_Out32(
            		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
            		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
            	);
            	i = 0;
            }
    	}
    }

    if (i != 0) {
        while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
        Xil_Out32(
    		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
    		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
    	);
    }

    _lcd_set_damaged(self, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void lcd_draw_dialog(
	struct Lcd* self,
	struct Pixel pix, struct Pixel pix_alt,
	int x1, int y1, int x2, int y2
) {
//	xil_printf(
//		"draw dialog: x1, y1, %d, %d, %d, %d\r\n",
//		x1, y1, x2, y2
//	);

    if (x1 > x2 || y1 > y2) return;

    struct _LcdColorImpl pix_impl = _lcd_color(&pix);
    struct _LcdColorImpl pix_alt_impl = _lcd_color(&pix_alt);

    _lcd_set_damaged(self, x1, y1, x2, y2);

    Xil_Out32(self->sys_gpio.BaseAddress, 0x01);

    u32 margin = 2;
    u32 boldness = 1;

    int i = 0;

    for (int y = y1; y <= y2; y += 1) {
    	for (int x = x1; x <= x2; x += 1) {
    		struct _LcdColorImpl _pix_impl = pix_alt_impl;

    		if (
				((y - y1) < margin) ||
				((x - x1) < margin) ||
				((y2 - y) < margin) ||
				((x2 - x) < margin)
			) {
//    			_pix_impl = pix_alt_impl;
    		} else if (
				((y - y1) >= margin && (y - y1) < (margin + boldness)) ||
				((x - x1) >= margin && (x - x1) < (margin + boldness)) ||
				((y2 - y) >= margin && (y2 - y) < (margin + boldness)) ||
				((x2 - x) >= margin && (x2 - x) < (margin + boldness))
			) {
				_pix_impl = pix_impl;
			}

            Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, _pix_impl.byte_higher);
            Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, _pix_impl.byte_lower);

            i += 1;

            if (i == LCD_DRAW_BLOCK_SIZE) {
                while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
                Xil_Out32(
            		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
            		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
            	);
            	i = 0;
            }
    	}
    }

    if (i != 0) {
        while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
        Xil_Out32(
    		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
    		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
    	);
    }

    _lcd_set_damaged(self, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void _font_render_engine_rawbb(
	struct Lcd* self,
	u32 x1, u32 y1,
	const font_t *fnt, const font_symbol_t *sym,
	struct _LcdColorImpl pix3_impl, struct _LcdColorImpl pix2_impl, struct _LcdColorImpl pix1_impl,
	struct _LcdColorImpl pix_alt_impl
) {
    uint8_t font_width = sym->width;
    uint8_t font_height = sym->height;

    uint8_t top = sym->margin_top;
    uint8_t bottom = font_height - sym->margin_bottom - 1;
    uint8_t left = sym->margin_left;
    uint8_t right = font_width - sym->margin_right - 1;

    uint16_t bi = sym->bmp_index;

//    xil_printf("print char idx %d, dim: %d, %d, margin: %d, %d, %d, %d\r\n",
//		bi, sym->width, sym->height,
//		sym->margin_top, sym->margin_bottom, sym->margin_left, sym->margin_right);

    _lcd_set_damaged(self, x1, y1, x1 + font_width - 1, y1 + font_height - 1);

    Xil_Out32(self->sys_gpio.BaseAddress, 0x01);

    uint8_t idx = 0;
    int i = 0;

    for(u32 h = 0; h < font_height; h += 1) {
        for(u32 w = 0; w < font_width; w += 1) {
            if(w < left || w > right || h < top || h > bottom) {
                // debug: lcdsim_write_gram(LCD_BLUE_COLOR);
                Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix_alt_impl.byte_higher);
                Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix_alt_impl.byte_lower);
            } else {
                uint8_t color_pixel = (fnt->bmp_base[bi] >> idx) & 0x03;
                if (color_pixel == 3) {
                    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix3_impl.byte_higher);
                    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix3_impl.byte_lower);
                } else if(color_pixel == 2) {
                    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix2_impl.byte_higher);
                    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix2_impl.byte_lower);
                } else if(color_pixel == 1) {
                    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix1_impl.byte_higher);
                    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix1_impl.byte_lower);
                } else {
                    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix_alt_impl.byte_higher);
                    Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix_alt_impl.byte_lower);
                }

                if(idx == 6) {
                	idx = 0;
                    bi += 1;
                } else {
                	idx += 2;
                }
            }

            i += 1;

            if (i == LCD_DRAW_BLOCK_SIZE) {
                while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
                Xil_Out32(
            		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
            		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
            	);
            	i = 0;
            }
        }
    }

    if (i != 0) {
        while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
        Xil_Out32(
    		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
    		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
    	);
    }

    _lcd_set_damaged(self, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

#define ROW_CLERANCE_SIZE 0

void lcd_draw_string(
	struct Lcd* self,
	struct Pixel pix, struct Pixel pix_alt,
	int x1, int y1,
	const char *s
) {
	struct Pixel pix3 = pix;
	struct Pixel pix2 = {
		.r = (((u32) pix3.r) * 5 + pix_alt.r) / 6,
		.g = (((u32) pix3.g) * 5 + pix_alt.g) / 6,
		.b = (((u32) pix3.b) * 5 + pix_alt.b) / 6,
	};
	struct Pixel pix1 = {
		.r = (((u32) pix3.r) * 4 + ((u32) pix_alt.r) * 2) / 6,
		.g = (((u32) pix3.g) * 4 + ((u32) pix_alt.g) * 2) / 6,
		.b = (((u32) pix3.b) * 4 + ((u32) pix_alt.b) * 2) / 6,
	};
    struct _LcdColorImpl pix3_impl = _lcd_color(&pix3);
    struct _LcdColorImpl pix2_impl = _lcd_color(&pix2);
    struct _LcdColorImpl pix1_impl = _lcd_color(&pix1);
    struct _LcdColorImpl pix_alt_impl = _lcd_color(&pix_alt);

    uint16_t orgx = x1;

    while((*s) != '\0') {
        if((*s) == '\r') {
            // no operation
        } else if((*s) == '\n') {
            y1 += (omori24.default_height + ROW_CLERANCE_SIZE);
            x1 = orgx;
        } else if((*s) == ' ') {
            x1 += omori24.default_width;
        } else {
        	font_symbol_t sym = {};
            if(!omori24.lookup((*s), &sym)) {
                x1 += omori24.default_width;
            } else {
            	_font_render_engine_rawbb(
            		self, x1, y1,
            		&omori24, &sym,
            		pix3_impl, pix2_impl, pix1_impl,
            		pix_alt_impl
            	);

                x1 += sym.width;
            }
        }
        s += 1;
    }
}

void lcd_draw_big_string(
	struct Lcd* self,
	struct Pixel pix, struct Pixel pix_alt,
	int x1, int y1,
	const char *s
) {
	struct Pixel pix3 = pix;
	struct Pixel pix2 = {
		.r = (((u32) pix3.r) * 5 + pix_alt.r) / 6,
		.g = (((u32) pix3.g) * 5 + pix_alt.g) / 6,
		.b = (((u32) pix3.b) * 5 + pix_alt.b) / 6,
	};
	struct Pixel pix1 = {
		.r = (((u32) pix3.r) * 4 + ((u32) pix_alt.r) * 2) / 6,
		.g = (((u32) pix3.g) * 4 + ((u32) pix_alt.g) * 2) / 6,
		.b = (((u32) pix3.b) * 4 + ((u32) pix_alt.b) * 2) / 6,
	};
    struct _LcdColorImpl pix3_impl = _lcd_color(&pix3);
    struct _LcdColorImpl pix2_impl = _lcd_color(&pix2);
    struct _LcdColorImpl pix1_impl = _lcd_color(&pix1);
    struct _LcdColorImpl pix_alt_impl = _lcd_color(&pix_alt);

    uint16_t orgx = x1;

    while((*s) != '\0') {
        if((*s) == '\r') {
            // no operation
        } else if((*s) == '\n') {
            y1 += (omori64.default_height + ROW_CLERANCE_SIZE);
            x1 = orgx;
        } else if((*s) == ' ') {
            x1 += omori64.default_width;
        } else {
        	font_symbol_t sym = {};
            if(!omori64.lookup((*s), &sym)) {
                x1 += omori64.default_width;
            } else {
            	_font_render_engine_rawbb(
            		self, x1, y1,
            		&omori64, &sym,
            		pix3_impl, pix2_impl, pix1_impl,
            		pix_alt_impl
            	);

                x1 += sym.width;
            }
        }
        s += 1;
    }
}

void lcd_draw_histogram(
	struct Lcd* self,
	struct Pixel pix, struct Pixel pix_alt,
	int x1, int y1,
	const u8 *s
) {
    struct _LcdColorImpl pix_impl = _lcd_color(&pix);
    struct _LcdColorImpl pix_alt_impl = _lcd_color(&pix_alt);

    int x2 = x1 + 200;
    int y2 = y1 + 128;

    _lcd_set_damaged(self, x1, y1, x2, y2);

    Xil_Out32(self->sys_gpio.BaseAddress, 0x01);

    int i = 0;

    for (int y = y1; y <= y2; y += 1) {
    	for (int x = x1; x <= x2; x += 1) {
    		struct _LcdColorImpl _pix_impl = pix_alt_impl;

    		if ((x - x1) % 2 == 0
				&& (y2 - y) < s[(x - x1) / 2] / 2) {
				_pix_impl = pix_impl;
			}

            Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, _pix_impl.byte_higher);
            Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, _pix_impl.byte_lower);

            i += 1;

            if (i == LCD_DRAW_BLOCK_SIZE) {
                while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
                Xil_Out32(
            		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
            		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
            	);
            	i = 0;
            }
    	}
    }

    if (i != 0) {
        while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
        Xil_Out32(
    		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
    		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
    	);
    }

    _lcd_set_damaged(self, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void lcd_clear(
	struct Lcd* self,
	int x1, int y1, int x2, int y2
) {
    if (x1 > x2 || y1 > y2) return;

    _lcd_set_damaged(self, x1, y1, x2, y2);

    Xil_Out32(self->sys_gpio.BaseAddress, 0x01);

    int i = 0;

    for (int y = y1; y <= y2; y += 1) {
    	for (int x = x1; x <= x2; x += 1) {
    		struct Pixel pix = {
				.r = lcd_background.pixel_data[(y * LCD_WIDTH + x) * 4],
				.g = lcd_background.pixel_data[(y * LCD_WIDTH + x) * 4 + 1],
				.b = lcd_background.pixel_data[(y * LCD_WIDTH + x) * 4 + 2],
    		};
    	    struct _LcdColorImpl pix_impl = _lcd_color(&pix);

            Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix_impl.byte_higher);
            Xil_Out32(self->sys_spi.BaseAddr + XSP_DTR_OFFSET, pix_impl.byte_lower);

            i += 1;

            if (i == LCD_DRAW_BLOCK_SIZE) {
                while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
                Xil_Out32(
            		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
            		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
            	);
            	i = 0;
            }
    	}
    }

    if (i != 0) {
        while (0 == (Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) & XSP_INTR_TX_EMPTY_MASK));
        Xil_Out32(
    		self->sys_spi.BaseAddr + XSP_IISR_OFFSET,
    		Xil_In32(self->sys_spi.BaseAddr + XSP_IISR_OFFSET) | XSP_INTR_TX_EMPTY_MASK
    	);
    }

    _lcd_set_damaged(self, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void lcd_clearall(struct Lcd* self) {
	lcd_clear(
		self,
		0, 0,
		LCD_WIDTH - 1, LCD_HEIGHT - 1
	);
}

void _lcd_handle_request(struct Lcd* self, struct DrawRequest* req) {
	switch (req->type) {
	case lcd_request_Rect:
		lcd_draw(
			self,
			req->pix,
			req->x1, req->y1,
			req->x2, req->y2
		);
		break;
	case lcd_request_Dialog:
		lcd_draw_dialog(
			self,
			req->pix, req->pix_alt,
			req->x1, req->y1,
			req->x2, req->y2
		);
		break;
	case lcd_request_String:
		lcd_draw_string(
			self,
			req->pix, req->pix_alt,
			req->x1, req->y1,
			(char*) req->aux_ptr
		);
		break;
	case lcd_request_Big_String:
		lcd_draw_big_string(
			self,
			req->pix, req->pix_alt,
			req->x1, req->y1,
			(char*) req->aux_ptr
		);
		break;
	case lcd_request_Histogram:
		lcd_draw_histogram(
			self,
			req->pix, req->pix_alt,
			req->x1, req->y1,
			(u8*) req->aux_ptr
		);
		break;
	case lcd_request_Clear:
		lcd_clear(
			self,
			req->x1, req->y1,
			req->x2, req->y2
		);
		break;
	case lcd_request_ClearAll:
		lcd_clearall(self);
		break;
	default:
		break;
	}
}

void lcd_handle_request(struct Lcd* self, struct DrawRequest* req) {
//	xil_printf("lcd:handling request...\r\n");

	microblaze_disable_interrupts(); // Because

	_lcd_handle_request(self, req);
	free(req);

	microblaze_enable_interrupts();
}

void lcd_draw_async(struct Lcd* self, struct DrawRequest* req) {
//	xil_printf("lcd:queuing request...\r\n");

	event_loop_push(
		self->event_loop,
		(SLOT_TYPE) lcd_handle_request,
		self, req
	);
}
