/*
 * Routines that control the LEDs
 */

/****** I N C L U D E S **********************************************************/
#include <xc.h>
#include <plib.h>
#include "HardwareConfig.h"
#include <stdlib.h>
#include <stdint.h>
#include "globals.h"
#include "libs/maxim6957.h"
#include "leds.h"

/** V A R I A B L E S ******************************************************/

/*
 Array of MAX6957 port numbers for the step LEDs.
 Due to circuit layout constraints the The LEDs are not connected
 sequentially to the MAX ports so we need to map from step index to 
 MAX port address.
 This allows us to quickly loop over the LEDs in a visually sequential order 
 Note: The last four are run,repeat,menu and BL
 */
const unsigned char g_maxPortMap[] = {
		12 // Step1
	,	13 // 2
	,	14 // 3
	,	21 // 4
	,	22 // 5
	,	27 // 6
	,	29 // 7
	,	31 // 8
	,	16 // 9
	,	17 // 10
	,	18 // 11
	,	20 // 12
	,	19 // 13
	,	23 // 14
	,	24 // 15
	,	25 // Step 16
	,	26 // Run
	,	28 // Repeat
	,	30 // Menu
	,	15 // LCD BL
};

/****** F U N C T I O N S **********************************************************/


// Sets the state [on|off] of a specific LED
void setLEDstate(lednames_t led, unsigned char state){
	max6957_setPortState(led+12, state);
}

/*
 * Sets the current (intensity) for an LED
 * We provide an LED name enum resulting in a zero based index
 * max6957_setSegCurrent() expects a port index starting ranging from 4 to 31
 * some version of the MAX6957 have more ports.
 * Our device only has 20 ports starting at port 12 so we must add an offset.
 */

void setLEDintensity(lednames_t led, unsigned char intensity) {
	if(intensity > LED_INTENSITY_MAX) return;
	max6957_setPortCurrent(led+12, intensity);
}

uint8_t getLEDintensity(lednames_t led) {
	return max6957_getPortCurrent(led+12);
}

/*
 * Turns all step LEDs off
 */
//void clearStepLED(){
//	//bulk write
//	max6957_write(MAX6957_ADDR_PG12_19, 0);
//	max6957_write(MAX6957_ADDR_PG20_27, 0);
//}


/*
 * Configures the max6957 the way we want it
 */
void max6957setup(void){
	uint8_t addr;
	//Config the Max

	max6957_config(0x00);

	max6957_config(MAX6957_SEGMENTCURRENT & MAX6957_NORMAL & MAX6957_TRANSOFF);
	//PDIP version has only ports 12 - 31
	
	//Set each port to zero : Use bulk port write
	for(addr = MAX6957_ADDR_PG12_19; addr <= MAX6957_ADDR_PG28_31; addr+=8){
		max6957_write(addr, 0x00);
	}
	
	// Configure all ports as LED
	for(addr = MAX6957_ADDR_PCONF3; addr <= MAX6957_ADDR_PCONF7; addr++){
		max6957_write(addr, 0x00);
	}


	//Test mode
	/*
	max6957_write(MAX6957_ADDR_TEST, 0xff);
	delay_ms(500);
	max6957_write(MAX6957_ADDR_TEST, 0x00);
	delay_ms(500);
	*/
	
	//Set each port to 1/16 current
	for(addr = MAX6957_ADDR_C13_12; addr <= MAX6957_ADDR_C31_30; addr++){
		max6957_write(addr, 0x00);
	}
	//Port28 is the run LED, it's green and needs a bit more current!
	//max6957_setSegCurrent(28, 0x07);
	
	//Do a loop lighting each LED in turn.
	for(addr = 0; addr <= 19; addr++){
		max6957_setPortState(g_maxPortMap[addr], 0x01);
		delay_ms(50);
	}
	delay_ms(200);
/*
	for(addr = 12; addr <= 31; addr++){
		max6957_setSegCurrent(addr, 7);
	}
	delay_ms(1000);

	for(addr = 12; addr <= 31; addr++){
		max6957_setSegCurrent(addr, 15);
	}
	delay_ms(1000);
*/	
	//Do a loop lighting each LED in turn.
	for(addr = 0; addr <= 19; addr++){
		max6957_setPortState(g_maxPortMap[addr], 0x00);
		delay_ms(30);
	}
	
	setLEDintensity(LCD_BL, 0x04);
	setLEDstate(LCD_BL, ON);
}

/*
 * Displays the programmed track for the current pattern
 * On the LEDS
 */
void led_displayPattern(void){
	uint16_t leds = g_song.data[g_song.pattern][g_song.track];
	int8_t idx = 0;
	//Can't bulk write as LEDs ports are not in sequence
	while(idx < 16){
		max6957_setPortState(g_maxPortMap[idx], leds & 0b1);
		//shift the leds
		idx++;
		leds >>= 1;
	};
}

/*
 * Displays the current step on the LEDs
 * LED is shown at high intensity compared
 * to current pattern which is shown as low intense
 * Step 0 is Max6957 P12
 * Display is static
 */
void led_displayStepNext(int8_t step){
	uint16_t prevstep = 0;
	uint16_t stepmask = 0b1;
	uint16_t pattern = g_song.data[g_song.pattern][g_song.track];
	step &= 0x0f; //We only have 16 LEDs so mask out anything higher
	//First clear the previous step LED back to it's programmed pattern state
	prevstep = (step - 1) & 0x0f;
	max6957_setPortCurrent(g_maxPortMap[prevstep], LED_INTENSITY_DEFAULT);
	//determine if previous step should be lit or not
	if(pattern & (stepmask << prevstep)){
		//Should be lit
		max6957_setPortState(g_maxPortMap[prevstep], 1);
	}else{
		//should be dark
		max6957_setPortState(g_maxPortMap[prevstep], 0);
	}
	//Now write the current step LED
	max6957_setPortCurrent(g_maxPortMap[step], LED_INTENSITY_BRIGHT);
	max6957_setPortState(g_maxPortMap[step], 1);
}

/*
 * Turns on the current step on the LEDs
 * LED is shown at high intensity compared
 * to current pattern which is shown as low intense
 * Step 0 is Max6957 P12
 * Use displayStepOff() to hide step LED
 */
void led_displayStepOn(int8_t step){
	step &= 0x0f; //We only have 16 LEDs so mask out anything higher
	//Now write the current step LED
	max6957_setPortCurrent(g_maxPortMap[step], LED_INTENSITY_BRIGHT);
	max6957_setPortState(g_maxPortMap[step], 1);
}
/*
 * Turns off the current step on the LEDs
 * Returns the LED to it's current pattern status
 * Step 0 is Max6957 P12
 * Partner to displayStepOn()
 */
void led_displayStepOff(int8_t step){
	uint16_t stepmask = 0b1;
	uint16_t pattern = g_song.data[g_song.pattern][g_song.track];
	step &= 0x0f; //We only have 16 LEDs so mask out anything higher
	max6957_setPortCurrent(g_maxPortMap[step], LED_INTENSITY_DEFAULT);
	//determine if previous step should be lit or not
	if(pattern & (stepmask << step)){
		//Should be lit
	}else{
		//should be dark
		max6957_setPortState(g_maxPortMap[step], 0);
	}
}


// Increases the LCD backlight intensity
uint8_t backlightInc( void ){
	uint8_t intens = getLEDintensity(LCD_BL);
	if(intens < LED_INTENSITY_MAX){
		intens++;
		setLEDintensity(LCD_BL, intens);
	}
	return intens;
}

// Decreases the LCD backlight intensity
uint8_t backlightDec( void ){
	uint8_t intens = getLEDintensity(LCD_BL);
	if(intens > LED_INTENSITY_MIN){
		intens--;
		setLEDintensity(LCD_BL, intens);
	}
	return intens;
}