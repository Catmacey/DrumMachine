
#ifndef _nokia5110lcd_H
#define _nokia5110lcd_H

/** D E F I N E S **********************************************************/

//Some LCD specific data no need to change
#define LCD_WIDTH	84
#define LCD_HEIGHT 48
#define LCD_ROWS 6
#define LCD_ALLBYTES (LCD_WIDTH * LCD_ROWS)
#define LCD_HARDWARERESET  //Define this if using R/C hardware reset
//Set this define to include the double height numbers
//#define LCD_USEBIGNUMBERS

#endif

