#ifndef _SPI_H
#define _SPI_H

/*
	Some simple blocking functions for using SPI
	Assumes SPI device 1

*/

/** D E F I N E S **********************************************************/
// The following PPS pins for module 1 must be defined somewhere.
// In hardwareconfig.h?
//#define SPI1_CLK 	PPS_RP8
//#define SPI1_DO 	PPS_RP9
//#define SPI1_DI 	PPS_RP10

//Define one of these to set the speed : Fallback if not defined is 250Khz
//#define SPI_SPEED_8MZ
//#define SPI_SPEED_4MZ
//#define SPI_SPEED_2MZ
//#define SPI_SPEED_1MZ
//#define SPI_SPEED_500KHZ
#define SPI_SPEED_FAST 8000000

#define SPI_FAST() SpiChnSetBitRate(SPI_CHANNEL2, GetPeripheralClock(), SPI_SPEED_FAST)
#define SPI_SLOW() SpiChnSetBitRate(SPI_CHANNEL2, GetPeripheralClock(), 100000)

/** I N C L U D E S **********************************************************/

/** P R O T O T Y P E S ******************************************************/

unsigned char SPI_8bitXchngDEBUG(unsigned char data);
void SPI_8bitWrite(unsigned char);
unsigned char SPI_8bitXchng(unsigned char);
unsigned char SPI_8bitRead(void);
void SPI_9bitWrite(unsigned char, unsigned char);
void softSPI_9bitWrite(unsigned char, unsigned char);

/*****************************************************************************
   V A R I A B L E S
******************************************************************************/

#endif

