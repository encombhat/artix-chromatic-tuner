/*
 * lcd_font.h
 *
 *  Created on: Oct 29, 2022
 *      Author: bhat
 */

#ifndef SRC_IO_LCD_FONT_H_
#define SRC_IO_LCD_FONT_H_


#include <stdint.h>
#include <stdbool.h>

// define the maximum size of margin top, margin bottom, margin left, margin width
#define FONT_MARGIN_DATABIT_SIZE        8
typedef uint32_t utf8_t;

//=================================================================

typedef struct
{
    uint8_t width;
    uint8_t height;

    // calc_margin
    uint8_t margin_top      :FONT_MARGIN_DATABIT_SIZE;
    uint8_t margin_bottom   :FONT_MARGIN_DATABIT_SIZE;
    uint8_t margin_left     :FONT_MARGIN_DATABIT_SIZE;
    uint8_t margin_right    :FONT_MARGIN_DATABIT_SIZE;

    uint16_t bmp_index;
}font_symbol_t;

//=================================================================


typedef bool (*fnt_lookup_fp)(utf8_t c, font_symbol_t *sym);

typedef struct {
    uint8_t default_width;
    uint8_t default_height;

    const uint8_t *bmp_base;

    fnt_lookup_fp lookup;
}font_t;

//=================================================================

extern font_t omori24;
extern font_t omori64;

#endif /* SRC_IO_LCD_FONT_H_ */
