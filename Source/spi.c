/*
* Very simple SPI library
*	Matt Casey Jun 2011
*	Assumes that we're using SPI module 2 in normal mode (not enhanced buffer)
*	This is for compat with other SPI devices
*/

/****** I N C L U D E S **********************************************************/
#include <xc.h>
#include <plib.h>
#include <stdint.h>
#include "HardwareConfig.h"
#include "SPI.h"

/****** V A R I A B L E S ********************************************************/


/*
*	Simple blocking 8bit SPI write on SPI2, /CS must be handled by caller.
*	SPI reads and write async so write always results in a read even if 
*	read data is dummy.
*/

void SPI_8bitWrite ( unsigned char data ){
	uint8_t tmp;
	SPI2BUF = data;
	while(SPI2STATbits.SPIBUSY);
	tmp = SPI2BUF;
}

//void SPI_8bitWrite ( unsigned char data ){
//	(void)SPI_8bitXchng(data);
//}

unsigned char SPI_8bitXchngDEBUG(unsigned char data){
	unsigned char tmp;
	//printf("\nSPI_8bitXchng(%2x){", data);
	//while(SPI2STATbits.SPIBUSY);
	SPI2BUF = data;
	while(!SPI2STATbits.SPIRBF);
	tmp = SPI2BUF;
	//printf("%2x}", tmp);
	return tmp;
}


unsigned char SPI_8bitXchng(unsigned char data){
	//while(SPI2STATbits.SPIBUSY);
	SPI2BUF = data;
	while(!SPI2STATbits.SPIRBF);
	//while(SPI2STATbits.SPIBUSY);
	return SPI2BUF;
}

/*
*	Simple blocking 8bit SPI read on SPI1, /CS must be handled by caller.
* It's the same as the Write command except that we send a dummy byte
* to clock in the read data. You could do that yourself using the write command...
*/
unsigned char SPI_8bitRead(void){
	return SPI_8bitXchng(0xff);
}

/*
 * Mostly hardware 9bit SPI.
 * First bit (D/C) is software
 */
void SPI_9bitWrite(unsigned char data, unsigned char ninth) {
	//Select LCD
	//LCD_CS_ENABLE();
	// Disable the SPI hardware to send the ninth bit
	uint8_t tmp;
	SpiChnEnable(SPI_CHANNEL2, 0);
	// Toggle clock
	SPI_CLK_LOW();
	if(ninth == 0) SPI_MOSI_LOW();
	else  SPI_MOSI_HIGH();
	SPI_CLK_HIGH();
	// Re-enable the SPI hardware
	SpiChnEnable(SPI_CHANNEL2, 1);
	//write data 8 bits
	SPI2BUF = data;
	while(SPI2STATbits.SPIBUSY);
	tmp = SPI2BUF;
}

/*
 * Software 9bit SPI
 */
void softSPI_9bitWrite(unsigned char data, unsigned char ninth) {
	unsigned char idx = 8;
	// Disable the SPI hardware to send the ninth bit
	SpiChnEnable(SPI_CHANNEL2, 0);
	//Select LCD
	LCD_CS_ENABLE();
	if(ninth == 0) SPI_MOSI_LOW();
	else  SPI_MOSI_HIGH();
	// Toggle clock
	SPI_CLK_LOW();
	_nop();_nop();_nop();_nop();
	SPI_CLK_HIGH();
	//write data 8 bits
	while(idx--){
	  if(data & 0b10000000) SPI_MOSI_HIGH();
		else SPI_MOSI_LOW();
		// Toggle clock
		SPI_CLK_LOW();
		_nop();_nop();_nop();_nop();
		data<<=1;
		SPI_CLK_HIGH();
	}
	LCD_CS_CLEAR();
	// Re-enable the SPI hardware
	SpiChnEnable(SPI_CHANNEL2, 1);
}