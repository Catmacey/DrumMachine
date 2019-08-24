/**
   \file graphics.c
   \brief Functions relating to graphics. e.g drawing lines, rectangles, circles etc.
   \author Andy Gock

   Some functions based on Limor Fried's PCD8544 Arduino library.

 */ 

/*
	Copyright (c) 2012, Andy Gock

	Copyright (c) 2012, Adafruit Industries

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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
//#include "font.h"

/* LCD screen buffer */
uint8_t lcd_xpos = 0; // Pixel X position
uint8_t lcd_ypos = 0; // Pixel Y position
uint8_t lcd_textMode = 0;



/*
 * Writes a character to the buffer
 * BOLD mode uses a simple OR'ing to make a bolder letter, is 1 px wider too
 * INVERSE draws white on black
 */

void lcd_putChar(uint8_t chr){
	uint8_t idx, jdx;
	uint16_t addr;

	if((chr<0x20) || (chr>0x84)) return;
	// Get the index of the first column
	addr = ((5*chr)-160);
//	if(lcd_textMode & BOLD){
//		tmp = 0;
//		if(lcd_textMode & INVERSE){
//			for(idx=5; idx>0; idx--){
//				lcd_buffer[lcd_index++] = ~(lcd_Font[addr] | tmp);
//				tmp = lcd_Font[addr++];
//			}
//			lcd_buffer[lcd_index++] = ~tmp;
//			lcd_buffer[lcd_index++] = 0xff;
//		}else{
//			for(idx=5; idx>0; idx--){
//				lcd_buffer[lcd_index++] = lcd_Font[addr] | tmp;
//				tmp = lcd_Font[addr++];
//			}
//			lcd_buffer[lcd_index++] = tmp;
//			lcd_buffer[lcd_index++] = 0x00;
//		}
//	}else{
		// Normal
		if(lcd_textMode & INVERSE){
			for(idx=5; idx>0; idx--){
				// Grab a column of data
				uint8_t data = lcd_Font[addr++];
				for(jdx=0; jdx<8; jdx++){
					lcd_set_pixel(lcd_xpos, lcd_ypos+jdx, !(data & (0b00000001 << jdx)));
				}
				lcd_xpos++;
			}
			// Empty col
			for(jdx=0; jdx<8; jdx++){
				lcd_set_pixel(lcd_xpos, lcd_ypos+jdx, 1);
			}
			lcd_xpos++;
		}else{
			for(idx=5; idx>0; idx--){
				// Grab a column of data
				uint8_t data = lcd_Font[addr++];
				for(jdx=0; jdx<8; jdx++){
					lcd_set_pixel(lcd_xpos, lcd_ypos+jdx, (data & (0b00000001 << jdx)));
				}
				lcd_xpos++;
			}
			// Empty col
			for(jdx=0; jdx<8; jdx++){
				lcd_set_pixel(lcd_xpos, lcd_ypos+jdx, 0);
			}
			lcd_xpos++;
		}
}
void lcd_putStr(const char *s){
//	printf("\nlcd_putStr(\"%s\")", s);
	while(*s) lcd_putChar(*s++);
}

void lcd_setTextMode(uint8_t mode){
	lcd_textMode = mode;
}

/*
 * Sends a custom icon.
 * expects to find them in a const called lcd_Icons
 * Icons are always 8x8 pixels
 * Writes them from the current cursor position
 */
void lcd_sendIcon(uint8_t num){
	uint8_t idx, jdx;
	uint16_t addr;
	if(num > LCD_ICONCOUNT) return;
	addr = num * 8;
	for(idx=8; idx>0; idx--){
		// Grab a column of data
		uint8_t data = lcd_Icons[addr++];
		for(jdx=0; jdx<8; jdx++){
			lcd_set_pixel(lcd_xpos, lcd_ypos+jdx, (data & (0b00000001 << jdx)));
		}
		lcd_xpos++;
	}
	// Empty col
	for(jdx=0; jdx<8; jdx++){
		lcd_set_pixel(lcd_xpos, lcd_ypos+jdx, 0);
	}
	lcd_xpos++;
}

/* Bresenham's algorithm - based on PCD8544 library Limor Fried */
void lcd_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
	uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
	uint8_t dx, dy, tick = 0, dotted = 0;
	int8_t err, ystep;

	if(color > 1) dotted = 1;

	//printf("\nlcd_draw_line(x0:%d, y0:%d, x1:%d, y1:%d, color:%d);", x0,y0,x1,y1,color);

	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}
	
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	//lcd_update_bbox( x0, y0, x1, y1 );

	dx = x1 - x0;
	dy = abs(y1 - y0);
	
	err = dx / 2;
	
	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}
	
	for (; x0<=x1; x0++) {
		if(dotted) color = ++tick&0b1;
		if (steep) {
			lcd_set_pixel(y0, x0, color);
		} else {
			lcd_set_pixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void lcd_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	int16_t i;
	for (i=x; i<x+w; i++) {
		int16_t j;
		for (j=y; j<y+h; j++) {
			lcd_set_pixel(i, j, color);
		}
	}
	//lcd_update_bbox(x, y, x+w-1, y+h-1);
}

void lcd_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	int16_t i;
	for (i=x; i<x+w; i++) {
		lcd_set_pixel(i, y, color);
		lcd_set_pixel(i, y+h-1, color);
	}
	for (i=y; i<y+h; i++) {
		lcd_set_pixel(x, i, color);
		lcd_set_pixel(x+w-1, i, color);
	} 
	//lcd_update_bbox(x, y, x+w-1, y+h-1);
}

void lcd_draw_rect_thick(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tx, uint8_t ty, uint8_t color)
{
	int16_t i, t;
	
	if (tx == 0) {
		tx = 1;
	}

	if (ty == 0) {
		ty = 1;
	}
	
	for (i=x; i<x+w; i++) {
		/* Top and bottom sides */
		for (t=0; t<(ty); t++) {
			lcd_set_pixel(i, y+t, color);
			lcd_set_pixel(i, y+h-1-t, color);
		}
	}
	for (i=y; i<y+h; i++) {
		/* Left and right sides */
		for (t=0; t<(tx); t++) {
			lcd_set_pixel(x+t, i, color);
			lcd_set_pixel(x+w-1-t, i, color);
		}
	} 
	//lcd_update_bbox(x, y, x+w-1, y+h-1);
}

void lcd_draw_rect_shadow(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	lcd_draw_rect(x, y, w, h, color);
	lcd_draw_line(x+1, y+h, x+w, y+h, color);
	lcd_draw_line(x+w, y+1, x+w, y+h, color);
}

void lcd_draw_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color)
{
		
	int8_t f = 1 - r;
	int8_t ddF_x = 1;
	int8_t ddF_y = -2 * r;
	int8_t x = 0;
	int8_t y = r;
	
	//lcd_update_bbox(x0-r, y0-r, x0+r, y0+r);
	
	lcd_set_pixel(x0, y0+r, color);
	lcd_set_pixel(x0, y0-r, color);
	lcd_set_pixel(x0+r, y0, color);
	lcd_set_pixel(x0-r, y0, color);
	
	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		lcd_set_pixel(x0 + x, y0 + y, color);
		lcd_set_pixel(x0 - x, y0 + y, color);
		lcd_set_pixel(x0 + x, y0 - y, color);
		lcd_set_pixel(x0 - x, y0 - y, color);
		
		lcd_set_pixel(x0 + y, y0 + x, color);
		lcd_set_pixel(x0 - y, y0 + x, color);
		lcd_set_pixel(x0 + y, y0 - x, color);
		lcd_set_pixel(x0 - y, y0 - x, color);
		
	}
}

void lcd_fill_circle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color)
{
	
	int8_t f = 1 - r;
	int8_t ddF_x = 1;
	int8_t ddF_y = -2 * r;
	int8_t x = 0;
	int8_t y = r;
	
	int16_t i;

	//lcd_update_bbox(x0-r, y0-r, x0+r, y0+r);
	
	for (i=y0-r; i<=y0+r; i++) {
		lcd_set_pixel(x0, i, color);
	}
	
	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		for (i=y0-y; i<=y0+y; i++) {
			lcd_set_pixel(x0+x, i, color);
			lcd_set_pixel(x0-x, i, color);
		} 
		for (i=y0-x; i<=y0+x; i++) {
			lcd_set_pixel(x0+y, i, color);
			lcd_set_pixel(x0-y, i, color);
		}    
	}
}

void lcd_invert_area(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t xx, yy;
	for (xx = x; xx < (x+w); xx++) {
		/* Loop through each partial column */
		for (yy = y; yy < (y+h); yy++) {
			/* Go down and invert every pixel */
			lcd_invert_pixel(xx,yy);
		}
	}
}

