/*
* Maxim 6957 SPI GPIO portexpander and LED driver
*	Matt Casey Jun 2011
*	Based on the handy datasheets.
* Has an internal 16bit shift reg so we need to write two byte for each /CS
*	Assumes that we're using SPI1 in 8bit mode (for compat with other SPI devices)
*/

/****** I N C L U D E S **********************************************************/
#include <xc.h>
#include <plib.h>
#include "HardwareConfig.h"
#include "SPI.h"
#include "maxim6957.h"

/****** V A R I A B L E S ********************************************************/

//Use a bit of ram to keep track of segment currents
//each nibble is represents a port, but we have to write a byte at a time.
//Defaults on power up to 0
unsigned char max6957_segcurrent[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*
*	Writes a 16bit word as two seperate bytes.
*	1st byte is command, 2nd is data 
*	Very simple "blocking" mode of operation, ie CPU waits for 
*	this to complete, interupts still fire of course.
*/
void max6957_write(unsigned char command, unsigned char data){
	command = command & 0b01111111; //Write commands have bit7 = 0
	MAX6957_CS = 0;
	SPI_8bitWrite(command);
	SPI_8bitWrite(data);
	MAX6957_CS = 1;
}

void max6957_config(unsigned char config){
	max6957_write(MAX6957_ADDR_CONFIG, config);
}

/*
	Sets a nibble in the segment current registers
	port >= 4 <= 31
	value >= 0 <= 15
	Uses max6957_segcurrent array to store current values
*/
void max6957_setSegCurrent(unsigned char port, unsigned char value){
	unsigned char offset = 0;
	//Validate port
	if(port > 31 || port < 4) return;
	value &= 0b00001111;
	//Work out which reg to write to
	offset = (port - 4) / 2;

	//prepare value
	if(port & 0b00000001){
		//Odd ports : high nibble
		value <<= 4;
 		max6957_segcurrent[offset] = (max6957_segcurrent[offset] & 0b00001111) | value;
	}else{
		//even ports : low nibble
		max6957_segcurrent[offset] = (max6957_segcurrent[offset] & 0b11110000) | value;
	}
	max6957_write(MAX6957_ADDR_C5_4 + offset, max6957_segcurrent[offset]);
}

