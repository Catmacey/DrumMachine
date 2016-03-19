/*
* Very simple SPI library
*	Matt Casey Jun 2011
*	Assumes that we're using SPI module 2 in normal mode (not enhanced buffer)
*	This is for compat with other SPI devices
*/

/****** I N C L U D E S **********************************************************/
#include <xc.h>
#include <plib.h>
#include "HardwareConfig.h"
#include "SPI.h"

/****** V A R I A B L E S ********************************************************/


/*
*	Simple blocking 8bit SPI write on SPI1, /CS must be handled by caller.
*	SPI reads and write async so write always results in a read even if 
*	read data is dummy.
*/

void SPI_8bitWrite ( unsigned char data ){
	unsigned char tmp;
	SPI2BUF = data;
	while(SPI2STATbits.SPIBUSY);
	tmp = SPI2BUF;
}

unsigned char SPI_8bitXchng(unsigned char data){
	SPI2BUF = data;
	while(SPI2STATbits.SPIBUSY);
	return SPI2BUF;
}

/*
*	Simple blocking 8bit SPI read on SPI1, /CS must be handled by caller.
* It's the same as the Write command except that we send a dummy byte
* to clock in the read data. You could do that yourself using the write command...
*/
unsigned char SPI_8bitRead(void){
	SpiChnPutC(SPICHANNEL, 0xff);
	return SpiChnGetC(SPICHANNEL);
}

