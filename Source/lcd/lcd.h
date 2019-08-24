/**
   \file lcd_graphics.h
   \brief Graphics routines
   \author Andy Gock
 */ 

/*
	Copyright (c) 2012, Andy Gock

	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of Andy Gock nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL ANDY GOCK BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Note that we don't write to the LCD in these commands only to the buffer
 */

#ifndef _LCD_H
#define _LCD_H
#include "../HardwareConfig.h"
#include <stdint.h>

#if defined(LCDTYPE_NOKIA_1202)
	#include "lcdtype/nokia1202LCD.h"
#elif defined(LCDTYPE_NOKIA_5110)
	#include "lcdtype/nokia5110LCD.h"
#elif defined(LCDTYPE_ITDB0218)
	#include "lcdtype/itdb02-1.8.h"
#else
	#error "Unknown LCD type"
#endif

#define swap(a, b) { uint8_t t = a; a = b; b = t; }

#include "font.h"
//#include "interface.h"

#define BLACK 1
#define WHITE 0

// Text modes
#define NORMAL  0b00000000
#define INVERSE 0b00000001
#define BOLD    0b00000010

/* Global variables used for GLCD library */
//extern uint8_t lcd_buffer[LCD_WIDTH * LCD_ROWS];

// These are used by the buffer functions to keep track of current position.
extern uint8_t lcd_xpos;
extern uint8_t lcd_ypos;

/* Utility */
void lcd_Init(void);

void lcd_render(void);

extern void delay_ms(uint16_t);

/* 
 * Prototype functions
 * The driver c file should implement these
 */

// Sets the cursor position : using width and height defined in font.h
void lcd_setCursor(uint8_t col, uint8_t row);
// Sets the cursor at any pixel column on a row
void lcd_setCursorXRow(uint8_t col, uint8_t row);
void lcd_renderLines(unsigned char, unsigned char);

/**
 * Set pixel to specified colour
 * \param x X-coordinate
 * \param y Y-coordinate
 * \param color Colour to set pixel
 * \see ColourConstants
 */
void lcd_set_pixel(uint8_t x, uint8_t y, uint8_t color);

/**
 * Get state of pixel from specified location
 * \param x X-coordinate
 * \param y Y-coordinate
 * \return Colour
 */
uint8_t lcd_get_pixel(uint8_t x, uint8_t y);

/**
 * Invert state of pixel of specified location
 * \param x X-coordinate
 * \param y Y-coordinate
 */
void lcd_invert_pixel(uint8_t x, uint8_t y);


//void lcd_cursorxy (unsigned char, unsigned char);
/* Clears only the buffer */
void lcd_clearBuffer();
/* Clears the buffer and write it to the screen */
void lcd_clearScreen();

// Writes a single character to the buffer
void lcd_putChar(uint8_t chr);
// Writes a single icon to the buffer
void lcd_sendIcon(uint8_t num);
// Writes a string to the buffer
void lcd_putStr(const char *s);
// Write a string inverted or bold
void lcd_setTextMode(uint8_t mode);
// Clears the line (set data to 0x00) : Leaves the cursor at the start of the cleared line
//void lcd_clearLine(unsigned char);

/**
 * Draw line
 * \param x0 Start x-coordinate
 * \param y0 Start y-coordinate
 * \param x1 End x-coordinate
 * \param y1 End y-coordinate
 * \param color Colour to set pixels
 * \see ColourConstants
 */
void lcd_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);

/**
 * Draw rectangle and fill with colour.
 * The border of the rectangle is the same as fill colour
 * \param x Start x-coordinate (left-most)
 * \param y Start y-coordinate (top-most)
 * \param w Width
 * \param h Height
 * \param color Colour to fill with
 * \see ColourConstants
 */
void lcd_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

/**
 * Draw rectangle but do not fill.
 * The border of the rectangle is the same as fill colour
 * \param x Start x-coordinate (left-most)
 * \param y Start y-coordinate (top-most)
 * \param w Width
 * \param h Height
 * \param color Colour of border
 * \see ColourConstants
 */
void lcd_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

/**
 * Draw rectangle but do not fill. User specified thickness.
 * The border of the rectangle is the same as fill colour
 * \param x Start x-coordinate (left-most)
 * \param y Start y-coordinate (top-most)
 * \param w Width (outermost pixels)
 * \param h Height
 * \param tx Thickness of horizontal border along X axis
 * \param ty Thickness of vertical border along Y axis
 * \param color Colour of border
 * \see ColourConstants
 */
void lcd_draw_rect_thick(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tx, uint8_t ty, uint8_t color);

/**
 * Draw rectangle but do not fill. Place a shadow line on the bottom-right of the window.
 * The border of the rectangle is the same as fill colour
 * \param x Start x-coordinate (left-most)
 * \param y Start y-coordinate (top-most)
 * \param w Width
 * \param h Height
 * \param color Colour of border
 * \see ColourConstants
 */
void lcd_draw_rect_shadow(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

/**
 * Draw circle but do not fill.
 * The border of the rectangle is the same as fill colour
 * \param x0 Centre x-coordinate (left-most)
 * \param y0 Centre y-coordinate (top-most)
 * \param r  Radius
 * \param color Colour of border
 * \see ColourConstants
 */
void lcd_draw_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);

/**
 * Draw circle and fill.
 * The border of the rectangle is the same as fill colour
 * \param x0 Centre x-coordinate (left-most)
 * \param y0 Centre y-coordinate (top-most)
 * \param r  Radius
 * \param color Colour of border
 * \see ColourConstants
 */
void lcd_fill_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color);

/**
 * Invert pixels in a retangular area.
 * \param x Start x-coordinate (left-most)
 * \param y Start y-coordinate (top-most)
 * \param w Width
 * \param h Height 
 */
void lcd_invert_area(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

#endif
