#ifndef _Config_H
#define _Config_H

/******* G E N E R I C   D E F I N I T I O N S ************************************************/

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 


#define	HIGH	1
#define	LOW	0
#define	OUTPUT	0
#define	INPUT 	1
//#define	SET	1
//#define	CLEAR	0

//#define GetSystemClock()  (40000000ul)
#define GetSystemClock()  (48000000ul)
//#define GetSystemClock()  (60000000ul)
//#define GetSystemClock()  (64000000ul)
#define GetPeripheralClock()    (GetSystemClock() / (1 << OSCCONbits.PBDIV))
#define TICKS_uS (GetSystemClock() / 1000000)/2   //sys clocks per micro second for core timer
#define TICKS_mS (GetSystemClock() / 1000)/2   //sys clocks per milli second for core timer


//Matrix Columns RB0 - 7
#define Col0 	_RB0
#define Col1 	_RB1
#define Col2 	_RB2
#define Col3 	_RB3
#define Col4 	_RB4
#define Col5 	_RB5
#define Col6 	_RB6
#define Col7 	_RB7

//Matrix Rows RA0 - 1
#define Row0 			_LATA1
#define Row1 			_LATA0
#define Row2 			_LATA2
#define Row0Tris 	_TRISA1
#define Row1Tris 	_TRISA0
#define Row2Tris 	_TRISA2

//PMW out
#define PWM _LATB9

//AT45 RAM CS
#define AT45_CS _LATB11

//Serial debug output
#define TX _LATB14
#define PPS_TX  PPS_RP14

//Spare GPIO (Used to be LCD D/C)
#define DAC_CS _LATA3

//SD Card
#define SD_CS _LATB10
#define SD_CS_ENABLE mPORTBClearBits(BIT_10);
#define SD_CS_CLEAR mPORTBSetBits(BIT_10);

#define SPICHANNEL 2

//NOKIA LCD PORT PIN DEFINTION
///****************************************************
// Hardware SPI
// Manual CS between each byte, but could be modified to do bulk transfers.
///****************************************************
#define LCD_CS 		_LATB12

#define LCD_CS_ENABLE() mPORTBClearBits(BIT_12)
#define LCD_CS_CLEAR() mPORTBSetBits(BIT_12)

#define SPI_CLK_HIGH() mPORTBSetBits(BIT_15)
#define SPI_CLK_LOW() mPORTBClearBits(BIT_15)

#define SPI_MOSI_HIGH() mPORTBSetBits(BIT_8)
#define SPI_MOSI_LOW() mPORTBClearBits(BIT_8)


//#define LCD_RST 	_LATB12 //Can be performed in hardware with R/C. saves pin

//#define LCD_BL 		_LATB9 //LED backlight PWM'd

//MAX6957
///****************************************************
// Hardware SPI
// Manual CS between each byte, but could be modified to do bulk transfers.
///****************************************************
#define MAX6957_CS 		_LATA4

#define LCDTYPE_NOKIA_1202


#endif
