
#ifndef _nokia1202lcd_H
#define _nokia1202lcd_H

//#include <stdint.h>

//Some LCD specific data no need to change
#define LCD_WIDTH	96
#define LCD_HEIGHT 68
#define LCD_COLUMNS 16 // For 6px width font
#define LCD_ROWS 9
#define LCD_ALLBYTES (LCD_WIDTH * LCD_ROWS)
#define LCD_HARDWARERESET  //Define this if using R/C hardware reset

#endif

