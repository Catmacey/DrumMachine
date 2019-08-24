/* 
 * File:   font.h
 * Author: Matt
 *
 * Created on 02 August 2013, 00:31
 *
 * Font and Icon definitions
 *
 */

#ifndef FONT_H
#define	FONT_H

#ifdef	__cplusplus
extern "C" {
#endif


#define FONT_WIDTH 6 // Width in pixels of a single fixed width column
#define FONT_HEIGHT 8 // Height in pixels of single row
#define LCD_ICONCOUNT 5
	
extern const char lcd_Icons[];
extern const char lcd_tinyBits[];
extern const char lcd_Font[];
	
#ifdef	__cplusplus
}
#endif

#endif	/* FONT_H */

