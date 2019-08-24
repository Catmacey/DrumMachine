/*
* Nokia 1202 SPI LCD 96x68
* Matt Casey JULY 2013
*	Based on the combined works of many others 
*	as well the handy datasheets.
*
*/

/****** I N C L U D E S *******************************************************/
#include <xc.h>
#include <plib.h>
#include "../../HardwareConfig.h"
#include "../../SPI.h"
#include "../lcd.h"

/****** V A R I A B L E S *****************************************************/
static uint8_t lcd_buffer[LCD_ALLBYTES];

/****** PRIVATE FUNCTION PROTOTYPES *******************************************/

static void _direct_setxy (unsigned char, unsigned char);
static void _direct_sety (unsigned char);
static void _writecommand(unsigned char command);
static void _writedata(unsigned char data);

/* 
*	Boot up code for the LCD
*/
void lcd_Init(void){
	#ifndef LCD_HARDWARERESET
	//GPIO driven reset : Why not save a pin and use a simple R/C circuit?
	LCD_RST = 0; 					// reset LCD
	delay_ms(100);      // Wait 100ms
	LCD_RST = 1;					// release from reset
	#endif

	_writecommand(0xE2);		// Reset
	delay_ms(10);
	_writecommand(0xA4);  //power save off
  _writecommand(0x2F);  //power control set
  _writecommand(0xB0);  //set page address
  _writecommand(0x10);  //set col=0 upper 3 bits
  _writecommand(0x00);  //set col=0 lower 4 bits
  _writecommand(0xAF);  //lcd display on
	//lcd_clearRam();			  		// Erase all pixel on the lcdram.
	//lcd_cursorxy(0,0);		   	// Cursor Home.
}

/* Based on PCD8544 library by Limor Fried */
void lcd_set_pixel(uint8_t x, uint8_t y, uint8_t color) {
	if (x > (LCD_WIDTH-1) || y > (LCD_HEIGHT-1)) {
		/* don't do anything if x/y is outside bounds of display size */
		return;
	}

	if (color) {
		/* Set black */
		lcd_buffer[x+ (y/8)*LCD_WIDTH] |= ( 1 << (y%8));
	} else {
		/* Set white */
		lcd_buffer[x+ (y/8)*LCD_WIDTH] &= ~ (1 << (y%8));
	}

	//lcd_update_bbox(x,y,x,y);
}

/* Based on PCD8544 library by Limor Fried */
uint8_t lcd_get_pixel(uint8_t x, uint8_t y) {
	if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) {
		return 0;
	}
	
	if ( lcd_buffer[x+ (y/8)*LCD_WIDTH] & ( 1 << (y%8)) ) {
		return 1;
	} else {
		return 0;
	}
}

void lcd_invert_pixel(uint8_t x, uint8_t y) {
	if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) {
		return;
	}
	//lcd_update_bbox(x,y,x,y);
	lcd_buffer[x+ (y/8)*LCD_WIDTH] ^= ( 1 << (y%8));
}


// Sets the cursor at a given character position (for this 8x6 font)
void lcd_setCursor(uint8_t col, uint8_t row){
	if((col < LCD_COLUMNS) && (row < LCD_ROWS)){
		lcd_xpos = col*6;
		lcd_ypos = row*8;
	}
	return;
}

/*
 * Puts ram pointer to X,Y column, row position
 * Input Arguments: x-> X cordinate range from 0 to 95
 * Input Arguments: y-> Y cordinate range from 0 to 8
 * Different to lcd_setCursor() because you can put the cursor at any column
 * not just at a character position
*/
void lcd_setCursorXRow(uint8_t col, uint8_t row){
	if((col < LCD_WIDTH) && (row < LCD_ROWS)){
		lcd_xpos = col;
		lcd_ypos = row*8;
	}
	return;
}


/*
 * Renders complete lines from the global buffer.
 *
 */
void lcd_renderLines(unsigned char from, unsigned char to){
	unsigned short cursor;
	if( (from > 8) || (to > 8) || (from > to)){
		return;
	}
	//place LCD cursor at the first column of the first row we want to write
  _direct_setxy(0, from);
	//Write buffer
	
	for(cursor=(from * LCD_WIDTH); cursor<((to+1)*LCD_WIDTH); cursor++){
		LCD_CS_ENABLE();
		SPI_9bitWrite(lcd_buffer[cursor], 1);
		LCD_CS_CLEAR();
	}
	
}
/*
 * Sends the entire buffer to the screen
 * For speed it does the whole thing here rather than calling SPI_9bitWrite()
 * Using SpiChnEnable takes 3.625uS per 9bitbyte (3.17mS for entire screen) at 4Mhz
 * Using SPI2CON.ON takes 3.312uS per 9bitbyte (2.87mS for entire screen) at 4Mhz or 1.71mS at 8Mhz
 */
void lcd_render(void){
	unsigned short idx;
	_direct_setxy(0,0);
	LCD_CS_ENABLE();
	for (idx = 0; idx < LCD_ALLBYTES; idx++){
		// Disable the SPI hardware to release the GPIO
		SPI2CONbits.ON = 0;
		// Toggle clock
		SPI_CLK_LOW();
		SPI_MOSI_HIGH();
		SPI_CLK_HIGH();
		// Re-enable the SPI hardware
		SPI2CONbits.ON = 1;
		//write data 8 bits
		SPI2BUF = lcd_buffer[idx];
		while(SPI2STATbits.SPIBUSY);
	}
	LCD_CS_CLEAR();
}


/*
 * Sends a graphic representation of a 16bit int
 * Uses a tiny font to cram all 16 bits in one row
 * Handy for displaying debug output
 * Can send MSBit first (dir=0) or LSBit first (dir=1)
 */
//void lcd_direct_bitGfx(unsigned short value, unsigned char dir){
//	unsigned char idx, jdx, offset;
//	unsigned short mask;
//	if(dir == 0){
//		//MSB first
//		mask = 0b1000000000000000;
//	}else{
//		//LSB first
//		mask = 0b0000000000000001;
//	}
//	LCD_CS_ENABLE();
//	for(idx=0; idx<16; idx++){
//		if(value & mask){
//			offset = 0;
//		}else{
//			offset = 4;
//		}
//		for(jdx=0; jdx<4; jdx++){
//			SPI_9bitWrite(lcd_tinyBits[offset++], 1);
//		}
//		SPI_9bitWrite(0, 1);
//		if(dir == 0){
//			value = value << 1;
//		}else{
//			value = value >> 1;
//		}
//	}
//	LCD_CS_CLEAR();
//}
//

/* Clears the buffer and renders it to the screen */
void lcd_clearScreen(void){
	memset(lcd_buffer, 0x00, LCD_ALLBYTES);
	lcd_render();
}

/* Clears the buffer and sets cursor to 0,0 */
void lcd_clearBuffer(void){
	memset(lcd_buffer, 0x00, LCD_ALLBYTES);
	lcd_xpos = 0;
	lcd_ypos = 0;
}


static void _writecommand(unsigned char command){
	LCD_CS_ENABLE();
	SPI_9bitWrite(command, 0);
	LCD_CS_CLEAR();
}

static void _writedata(unsigned char data){
	LCD_CS_ENABLE();
	SPI_9bitWrite(data, 1);
	LCD_CS_CLEAR();
}

/*
 * Sets Electronic Volume for VLCD regulator
 * Contrast : 0 - 31
 */
void lcd_contrast(unsigned char contrast){
  contrast &= 0x1F;
  _writecommand(0x80 + contrast);
}
/* 
	Puts ram pointer to X,Y column, row position            
	Input Arguments: x-> X cordinate range from 0 to 95 
	Input Arguments: y-> Y cordinate range from 0 to 8  
*/ 
static void _direct_setxy (unsigned char x, unsigned char y){
  _writecommand(0xB0 | (y & 0x0F));
  _writecommand(0x10 | ((x >> 4) & 0x07));
  _writecommand(0x00 | (x & 0x0F));
}

/* 
	Puts ram pointer to Y row position            
	Input Arguments: y-> Y cordinate range from 0 to 8  
*/ 
void _direct_sety (unsigned char y){
	_writecommand(0xB0 | (y & 0x0F));
}
