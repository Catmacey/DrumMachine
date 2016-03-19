/*
	Header for Maxim 6957
	4-Wire-Interfaced, 2.5V to 5.5V, 20-Port and
	28-Port LED Display Driver and I/O Expander
	28 pin SPDIP package has 20 ports (P12 - P31)
	36/44 pin SSOP/QFN have 28 ports (P4 - P31)
	More info : http://www.maxim-ic.com/datasheet/index.mvp/id/3365
*/


#ifndef _maxim6957_H
#define _maxim6957_H

/** D E F I N E S **********************************************************/

//The following must be defined somewhere. In hardwareconfig.h?
//#define MAX6957_CS 		_LATXX

//register addresses
//Bit 7 is Read/Write 1 = Read, 0 = Write

#define MAX6957_ADDR_CONFIG			0x04 //Configuration reg
//Config register values. AND them together
#define MAX6957_GLOBALCURRENT 	0b10111111
#define MAX6957_SEGMENTCURRENT 	0b11111111
#define MAX6957_SHUTDOWN				0b11111110
#define MAX6957_NORMAL					0b11111111
#define MAX6957_TRANSON					0b11111111
#define MAX6957_TRANSOFF				0b01111111

//Test mode 0x00 = off, 0xff = on
#define	MAX6957_ADDR_TEST				0x07 

/*
	Port configuration registers
	4 ports per reg, 2 bits each
	0b00 = LED segment driver
	0b01 = GPIO Output
	0b10 = GPIO Input without pullup
	0b11 = GPIO Input with pullup
*/
//only on higer pin count packages
#define	MAX6957_ADDR_PCONF1			0x09 //P7 - P4
#define	MAX6957_ADDR_PCONF2			0x0A //P11 - P8
//All packages
#define	MAX6957_ADDR_PCONF3			0x0B //P15 - P12
#define	MAX6957_ADDR_PCONF4			0x0C //P19 - P16
#define	MAX6957_ADDR_PCONF5			0x0D //P23 - P20
#define	MAX6957_ADDR_PCONF6			0x0E //P27 - P24
#define	MAX6957_ADDR_PCONF7			0x0F //P31 - P28

//Individual port registers : Only data bit 0 is used
//First 4 ports are virtual
#define	MAX6957_ADDR_P0					0x20 
#define	MAX6957_ADDR_P1					0x21
#define	MAX6957_ADDR_P2					0x22
#define	MAX6957_ADDR_P3					0x23
//First real port : Only in high pin count
#define	MAX6957_ADDR_P4					0x24
#define	MAX6957_ADDR_P5					0x25
#define	MAX6957_ADDR_P6					0x26
#define	MAX6957_ADDR_P7					0x27
#define	MAX6957_ADDR_P8					0x28
#define	MAX6957_ADDR_P9					0x29
#define	MAX6957_ADDR_P10				0x2A
#define	MAX6957_ADDR_P11				0x2B
//All packages
#define	MAX6957_ADDR_P12				0x2C
#define	MAX6957_ADDR_P13				0x2D
#define	MAX6957_ADDR_P14				0x2E
#define	MAX6957_ADDR_P15				0x2F
#define	MAX6957_ADDR_P16				0x30
#define	MAX6957_ADDR_P17				0x31
#define	MAX6957_ADDR_P18				0x32
#define	MAX6957_ADDR_P19				0x33
#define	MAX6957_ADDR_P20				0x34
#define	MAX6957_ADDR_P21				0x35
#define	MAX6957_ADDR_P22				0x36
#define	MAX6957_ADDR_P23				0x37
#define	MAX6957_ADDR_P24				0x38 
#define	MAX6957_ADDR_P25				0x39
#define	MAX6957_ADDR_P26				0x3A
#define	MAX6957_ADDR_P27				0x3B
#define	MAX6957_ADDR_P28				0x3C
#define	MAX6957_ADDR_P29				0x3D
#define	MAX6957_ADDR_P30				0x3E
#define	MAX6957_ADDR_P31				0x3F

/*
	Port group registers : Just the useful ones really
	Write or read 8 ports at a time.
*/
#define MAX6957_ADDR_PG4_11			0x44
#define MAX6957_ADDR_PG12_19		0x4C
#define MAX6957_ADDR_PG20_27		0x54
#define MAX6957_ADDR_PG28_31		0x5C //Only 4 bits : bits 0-3.

/*
	Individual port current registers : Each reg is two ports
	4 bits each. A write will modify both ports so it's up to
	you to keep a track of the current value of the other port
	so you can set it to the same value.
*/
//These ports are only on high pin count packages
#define MAX6957_ADDR_C5_4				0x12
#define MAX6957_ADDR_C7_6				0x13
#define MAX6957_ADDR_C9_8				0x14
#define MAX6957_ADDR_C11_10			0x15
//All packages
#define MAX6957_ADDR_C13_12			0x16
#define MAX6957_ADDR_C15_14			0x17
#define MAX6957_ADDR_C17_16			0x18
#define MAX6957_ADDR_C19_18			0x19
#define MAX6957_ADDR_C21_20			0x1A
#define MAX6957_ADDR_C23_22			0x1B
#define MAX6957_ADDR_C25_24			0x1C
#define MAX6957_ADDR_C27_26			0x1D
#define MAX6957_ADDR_C29_28			0x1E
#define MAX6957_ADDR_C31_30			0x1F

/** I N C L U D E S **********************************************************/

/** P R O T O T Y P E S ******************************************************/
void max6957_write(unsigned char command, unsigned char data);
void max6957_config(unsigned char config);
void max6957_setSegCurrent(unsigned char port, unsigned char value);

/*****************************************************************************
   V A R I A B L E S
******************************************************************************/

#endif

