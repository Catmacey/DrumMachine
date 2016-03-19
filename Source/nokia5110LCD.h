
#ifndef _nokia5110lcd_H
#define _nokia5110lcd_H

/** D E F I N E S **********************************************************/

//The following must be defined somewhere. In hardwareconfig.h?
//#define LCD_CLK 	_LATBX
//#define LCD_DAT 	_LATBX
//#define LCD_DC 		_LATBX
//#define LCD_RST 	_LATBX
//#define LCD_CS 		_LATBX //Not really needed if LCD is only device on bus but makes debugging easier
//#define LCD_BL 		_LATBX //LED backlight if wanted. PWM'd is best

//Some LCD specific data no need to change
#define LCD_WIDTH	84
#define LCD_HEIGHT 48
#define LCD_ALLBYTES ((LCD_WIDTH * LCD_HEIGHT) / 8)
#define LCD_HARDWARERESET  //Define this if using R/C hardware reset
//Set this define to include the double height numbers
//#define LCD_USEBIGNUMBERS


/** I N C L U D E S **********************************************************/

/** P R O T O T Y P E S ******************************************************/
void lcd_write(unsigned char);
void lcd_writecommand(unsigned char);
void lcd_writedata(unsigned char);
void lcd_cursorxy (unsigned char, unsigned char);
void lcd_setxy (unsigned char, unsigned char);
void lcd_sety (unsigned char);
void lcd_clearRam(void);
void lcd_Init(void);
void lcd_sendChar(unsigned char);
void lcd_sendIcon(unsigned char);
void lcd_sendStr(const char *);
void lcd_sendHex(unsigned char);
void lcd_clearLine(unsigned char);
void lcd_fillscreen(void);
void lcd_sendCharBig(unsigned char, unsigned char);
void lcd_sendStrBig(const char *, unsigned char, unsigned char);
void lcd_sendBitGfx(unsigned short, unsigned char);
void lcd_renderLines(unsigned char, unsigned char);

/*****************************************************************************
   V A R I A B L E S
******************************************************************************/

extern volatile unsigned char g_lcdBuffer[];



#endif

