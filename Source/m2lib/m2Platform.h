
/*
 *
 *
 * Menu2 v2.0
 * Martin Hubacek
 * 18.3.2013
 * http://martinhubacek.cz
 *
 *
 */


#include "../lcd/lcd.h"

// Set rows/cols based on your font (for graphical displays)
#define ROW(x) (x)
#define COL(y) (y)

// Number of items on one screen
// Not including title
#define MENU_LINES 8

// Symbol which is displayed in front of the selected item
// This symbol doesn't appear when MENU_LINES == 1
#define ARROW_SYMBOL ">"
// How many spaces is between arrow symbol and menu item
// useful to set zero on smaller displays
#define ARROW_GAP 1

// Clear display
#define displayClear()	lcd_clearBuffer()

// Display string
//#define displayString(str, posx, posy) lcdBufferString(str, posx, posy)
// If you have separate functions for set position and print text, use define below
#define displayString(str, posx, posy) {lcd_setCursor(posx, posy); lcd_putStr(str);}

// Display number
//#define displayNumber(str, posx, posy) lcdBufferNumber(str, posx, posy)
// If you have separate functions for set position and print number, use define below
#define displayNumber(str, posx, posy) {lcd_setCursor(posx, posy); xprintf("%d", str);}

//#define displayFormatted(menuItem, posx, posy) {lcd_setCursor(posx, posy); xprintf(menuItem->text, (uint8_t)*((int*)menuItem->parameter));}

// Optional function to write buffer to display - comment if not used
#define displayDraw()		lcd_render()


#define MENU_MS_TICK msTick
extern volatile unsigned long msTick;
