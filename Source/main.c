/*
 * PIC32MX150F128B
 * Sample player, sequencer
 * Samples are 8bit signed @16khz
 * Uses MAX6957 to drive LEDs
 * Single 156Khz channel PWM for audio
 * 4 sample polyphony
 * 2x8 matrix for input
 * Loads/Saves up to 16 songs named "song_1.txt" -> "song_16.txt"
*/
#include <xc.h>
#include <plib.h>

// Configuring the Device Configuration Registers
#pragma config JTAGEN = OFF         // Disable JTAG
#pragma config DEBUG    = ON           // Background Debugger
#pragma config ICESEL  = ICS_PGx2       // ISCP Debug pair
#pragma config FWDTEN = OFF             // WD timer: OFF
#pragma config POSCMOD = OFF             // Primary Oscillator Mode
#pragma config FSOSCEN = OFF        // Disable secondary oscillator
#pragma config FPBDIV = DIV_1           // Peripheral Bus Clock: Divide by 1
#pragma config BWP = OFF                // Boot write protect: OFF
#pragma config FNOSC = FRCPLL   // Internal Fast RC oscillator (8 MHz) w/ PLL
#pragma config FPLLIDIV = DIV_2 // Divide FRC before PLL (now 4 MHz)
#pragma config FPLLMUL = MUL_20 // PLL Multiply (now 80 MHz)
#pragma config FPLLODIV = DIV_2 // Divide After PLL (now 40 MHz)


#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "xprintf.h"
#include "diskio.h"
#include "ff.h"

#include "splash.h"


//See inside HardwareConfig.h for pin defs
#include "HardwareConfig.h"
#include "globals.h"
#include "SPI.h"
#include "nokia5110lcd.h"
#include "maxim6957.h"
#include "song_saveload.h"

//Samples
#include "drumkit.h"

/*
 *	Function prototypes
 */
 
void timer1Init(void);
void timer2Init(void);
void initpic(void);
void bootUp(void);
void max6957setup(void);
void lcdBLOn(void);
void lcdBLOff(void);
void displayStepNextLEDs(int8_t);
void clearStepLED();
void setupSampleTracks(void);
void displayPatternLEDs(void);
void displayPatternTrackCounters(void);
void displayBPM(void);
void displayTrackVolumes(void);
void displayPatternGraph(void);
void seq_run(void);
void seq_stop(void);
void seq_toggleRepeat(void);
void seq_toggle(void);
void toggleMenu(void);
void displayStepOn(int8_t);
void displayStepOff(int8_t);
void patternNext(void);
void patternPrev(void);
void trackPrev(void);
void trackNext(void);
void bpmUp(void);
void bpmDown(void);
void bpmSetTmr(void);
void songLenInc(void);
void songLenDec(void);
void printBits(size_t const, void const * const);
void displayRunScreen(void);
void displayStep(void);
//void delay_ms(uint16_t);
InputRingBuffer_t gatherInput( void );

//#include "uart1.h"

int main(void){
	InputRingBuffer_t input, previnput;
	uint8_t idx = 0;
	uint8_t patternIdx = 15;
	uint8_t trackIdx = 15;
	uint8_t step = 15;
	uint8_t stepon = 0;
	uint8_t bpm = 0;
	MenuMode_t menuMode = g_menuMode;
	int8_t tdx = 0;
	uint16_t rdx = 0;
	
	bootUp();

  _LATB14 = 1;

	//Set default output for prints
	xdev_out(lcd_sendChar);

	lcdBLOn();

	lcd_cursorxy(0,0);
	lcd_sendStr("Boot");

	g_systemState.complete = 0x0000;

	//NOTE set g_systemState.trackChange to trigger display of track/step on first run rather than
	//Current technique of setting pattern step and tracks to false values (15)

	//Stop the sequencer
	seq_stop();
	bpmSetTmr();

	lcd_cursorxy(0,0);
	lcd_sendStr("Drumkit Ready");

	for(rdx=0; rdx<LCD_ALLBYTES; rdx++){
		g_lcdBuffer[rdx] = splash[rdx];
	}
	lcd_renderLines(0,5);
	rdx = 0;

	// Debug : Clear terminal screen
	puts("\033[2J");
	printf("Ready to go!");
  _LATB14 = 1;

	//preload g_input to avoid accidental triggering on reset
	for(idx=0; idx<INPUTRINGLEN; idx++){
		g_input[idx].complete = 0xffffffff;
	}
	previnput.complete = 0x00000000;

#ifdef __DEBUG
		//Debug : Add some dummy pattern
		g_song.data[0][0] = 0b1111111111111111;
		g_song.data[0][1] = 0b1010101010101010;
    g_song.data[0][2] = 0b0101010101010101;
    g_song.data[0][3] = 0b1111000011110000;
		g_systemState.repeat = 0b1;
		seq_run();
	#endif

	//Setup silly volume mode
	for(idx=0; idx<=TRACKCOUNT; idx++){
		g_song.voldir[idx] = 1;
	}

  lcd_sendStr(" OK");

	tdx = VOLUME_MIN;

	for(rdx=0; rdx<LCD_ALLBYTES; rdx++){
		g_lcdBuffer[rdx] = 0b10010001;
		//lcd_writedata();
	}

	//Setup zeroed volumes for each track
	for(idx=0; idx <= TRACKCOUNT; idx++){
		g_song.volume[idx] = 0;
	}

  _LATB14 = 0;
//
//	idx = 0;
//  while(1){
//    _LATB14 = ~_LATB14;
//    lcd_sendStr("=-_");
//		//SpiChnPutC(SPICHANNEL, idx++);
////		lcd_write(idx++);
//		delay_ms(50);
//  }

	//Enable Timer 1 interupt : Handles PWM loading and sample mixing
	EnableIntT1;
	//Enable Timer 4 interupt : Handles input
	EnableIntT4;


	/*
	//Test JSON stuff
	json = cJSON_Parse(jsontest);
	//if (!json) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
	key = cJSON_GetObjectItem(json,"key");
	xprintf("KEY type=%u value=\"%u\"\n", key->type, key->valueint);
	key = cJSON_GetObjectItem(json,"pattern");
	xprintf("PATTERN type=%u len=%u\n", key->type, cJSON_GetArraySize(key));
	xprintf("Data[1]\"%s\"\n", cJSON_GetArrayItem(key, 1)->valuestring);
	*/
	/*
	idx = 10;
	while(idx--){
		//xfprintf(&UART1PutChar, "Ready %d \n", idx);
		delay_ms(250);
	}
	*/

	lcd_clearRam();

	//Main
	while(1){

		//mPORTBToggleBits(BIT_14);

		//Process the input ring buffer
		input = gatherInput();

		if(input.complete != previnput.complete){
			/*
			 * State is different to last time!
			 * Do something!
			 */

			/*
			lcd_cursorxy(0,3);
			lcd_sendBitGfx(input.steps, 1);
			lcd_cursorxy(0,4);
			lcd_sendBitGfx(input.buttons, 1);
			*/
			//Important stuff first
			if(input.run) 		seq_toggle();
			if(input.menu) 		toggleMenu();
			if(input.repeat) 	seq_toggleRepeat();

			if(g_systemState.menu){
				//Joystick and step buttons do something different in menu mode
				switch(g_menuMode){
					case MENU_SONGLENGTH :{
						if(input.right)	songLenInc();
						if(input.left)	songLenDec();
						break;}
					case MENU_SAVESONG :{
						//For now we just save. at some point we'll need a menu or something to ask where
						if(input.right){
							saveSong();
							menuMode = MENU_DEFAULT;
						}
						break;}
					case MENU_LOADSONG :{
						//For now we just load. at some point we'll need a menu or something to ask where
					  if(input.right){
							loadSong();
							menuMode = MENU_DEFAULT;
							toggleMenu();
						}
						break;}
					case MENU_JAMMODEOFF :
					case MENU_JAMMODEON :{
						if(input.right){
							if(g_systemState.jamming){
								g_systemState.jamming = 0;
								menuMode = MENU_JAMMODEOFF;
							}else{
								g_systemState.jamming = 1;
								menuMode = MENU_JAMMODEON;
							}
							lcd_clearLine(1);
							lcd_cursorxy(0,1);
							lcd_sendStr(g_menuModeTitle[menuMode]);
						}
						break;
					}
					default :{
						break;}
				}
			}else{
				if(input.right)			trackNext();
				if(input.left)		trackPrev();
				if(g_systemState.running){
					//Things we can do with running
					if(input.up) bpmUp();
					if(input.down)	bpmDown();
				}else{
					//Things to do whilst stopped
					if(input.up) patternNext();
					if(input.down)	patternPrev();
				}
			}
			
			/*
			 * Dealing with the step buttons
			 */
			
			if(input.steps > 0 && input.steps != previnput.steps){
				if(g_systemState.menu){
					//In menu mode step buttons do other stuff
					if(input.songlength){
						menuMode = MENU_SONGLENGTH;
					}
					if(input.savesong){
						menuMode = MENU_SAVESONG;
					}
					if(input.loadsong){
						menuMode = MENU_LOADSONG;
					}
					if(input.jamming){
						if(g_systemState.jamming){
							menuMode = MENU_JAMMODEON;
						}else{
							menuMode = MENU_JAMMODEOFF;
						}
					}

					if(menuMode == g_menuMode){
						//Same button pressed again : Set menu to default
						g_menuMode = MENU_DEFAULT;
					}else{
						//Set menu mode
						g_menuMode = menuMode;
					}
					//xfprintf(&UART1PutChar, "\nMenu %d:\"%s\"", g_menuMode, g_menuModeTitle[g_menuMode]);

					if(g_menuMode == MENU_DEFAULT){
						lcd_clearLine(1);
						lcd_clearLine(2);
					}else{
						lcd_clearLine(1);
						lcd_clearLine(2);
						lcd_cursorxy(0,1);
						lcd_sendStr(g_menuModeTitle[g_menuMode]);
					}

					// Need to work out a nice loop or something to handle a "radio button"
					// effect on these modes. Store the old mode from last time and compare
					// it to this time. If they are then same then set default mode

				}else{
					//Step buttons set the beats
					g_song.data[g_song.pattern][g_song.track] ^= input.steps;
					if(!g_systemState.running){
						//play note if not running
						g_channel[0].position = 0;
						g_channel[0].instrument = g_song.track;
					}
					g_systemState.trackchange = 1;
				}
			}
			
			//Copy state over to prevstate for comparison next iter
			previnput = input;
		}//if

		/*
			Every loop
		*/

	/*
		lcd_cursorxy(0,0);
		//output the channel stack
		for(idx=0; idx<=CHANNELCOUNT; idx++){
			xprintf("C%u:%u ", idx, g_channel[idx].sample);
		}//for
		*/


		/*
		if(g_systemState.menu){
			//Display menu things maybe?


		}else{
		*/
			//Display pattern/track data
			if(
					patternIdx != g_song.pattern
					|| trackIdx != g_song.track
					|| g_systemState.trackchange
			){
				//Display the current pattern/track on the LEDs
				patternIdx = g_song.pattern;
				trackIdx = g_song.track;
				displayPatternLEDs();
				displayPatternTrackCounters();
				displayPatternGraph();
//				lcd_cursorxy(0,0);
//				xprintf("P%2u     \n", patternIdx+1);
//				lcd_cursorxy(0,1);
//				lcd_sendIcon(0);
//				xprintf("%2u\n", (int8_t)trackIdx+1);
//				lcd_sendStr(g_sampTitle[trackIdx]);
				g_systemState.trackchange = 0;
			}
		//}
		//Display BPM
		if(bpm != g_song.bpm){
			bpm = g_song.bpm;
			displayBPM();
//			lcd_cursorxy(9,0);
//			lcd_sendIcon(1);
//			xprintf("%3u\n", bpm);
		}

		if(step != g_song.step){
			step = g_song.step;
			//Step change
			if(!g_systemState.menu){
				displayStep();
			}
			//Update the LEDs if running
			if(g_systemState.running){
				displayStepOn(step);
				stepon = 1;
			}
			if(g_systemState.jamming){
				//Display the current track volumes
				displayTrackVolumes();
			}
		}
		if(stepon && (TMR2 > 3125)){
			//relies upon tmr2 being reset on each step
			//3125 is 50ms at TMR rate
			displayStepOff(step);
			stepon = 0;
		}
		//mPORTBToggleBits(BIT_14);
		delay_ms(5);
	}//while

}//func

void displayRunScreen(void){
	displayPatternLEDs();
	displayPatternTrackCounters();
	displayPatternGraph();
	displayBPM();
	displayStep();
}

void displayStep(void){
	lcd_cursorxy(4,0);
	lcd_sendIcon(3);
	xprintf("%2u\n", g_song.step+1);
}

//Displays the current pattern and track at fixed locations on the LCD
void displayPatternTrackCounters(void){
	lcd_cursorxy(0,0);
	lcd_sendIcon(4);
	xprintf("%2u\n", g_song.pattern+1); //+1 to make it human
	lcd_cursorxy(0,1);
	lcd_sendIcon(0);
	xprintf("%2u\n", (int8_t)g_song.track+1);  //+1 to make it human
	lcd_sendStr(g_sampTitle[g_song.track]);
}

//Displays the current BPM at a fixed location on the LCD
void displayBPM(void){
	lcd_cursorxy(9,0);
	lcd_sendIcon(1);
	xprintf("%3u\n", g_song.bpm);
}

//Displays the complete pattern, all 16 tracks all 16 steps
void displayPatternGraph(void){
	uint8_t tdx, sdx, offset, mask;
	uint16_t pattern, cursor;

	//Clear the area (lines 2 to 5) that we want to draw into
	cursor = 0;
	offset = 2;
	for(tdx=2; tdx<6; tdx++){
		cursor = tdx*LCD_WIDTH;
		for(sdx=0; sdx<LCD_WIDTH; sdx++){
			if((sdx >= offset) && (sdx % 5 == (2+offset))){
				g_lcdBuffer[cursor++] = 0b10101010;
			}else{
				g_lcdBuffer[cursor++] = 0;
			}
		}
	}
	for(tdx=0; tdx<=g_trackCount; tdx++){
		pattern = g_song.data[g_song.pattern][tdx];
		mask = 0b00000011 << (tdx % 4)*2;
		cursor = (LCD_WIDTH * ((tdx/4)+2)) + offset; //Start position
		for(sdx=0; sdx<=g_song.patternLength; sdx++){
			if(pattern & 0b0000000000000001){
				//selected
				g_lcdBuffer[cursor+1] = g_lcdBuffer[cursor+1] | mask;
				g_lcdBuffer[cursor+2] = g_lcdBuffer[cursor+2] | mask;
				g_lcdBuffer[cursor+3] = g_lcdBuffer[cursor+1];
 			}
			cursor+=5;
			pattern = pattern >> 1;
		}
	}
	//Write the buffer to the LCD
	lcd_renderLines(2,5);

//	lcd_setxy(0,2);
//	for(cursor = (LCD_WIDTH * 2); cursor<LCD_ALLBYTES; cursor++){
//		lcd_writedata(g_lcdBuffer[cursor]);
//	}

}


//Displays the current track volumes as 16 columns of 4 rows (32px)
 void displayTrackVolumes(void){
	//volatile uint8_t data[4*LCD_WIDTH]; //this is the screen buffer (bit wasteful)
	volatile uint8_t tdx = 0;
	volatile uint8_t rdx = 0;
	volatile uint8_t wdx = 0;
	volatile uint16_t cursor = 0;
	volatile uint8_t volume = 0;
	volatile uint32_t volbits = 0b10000000000000000000000000000000;
	volatile uint8_t chunk = 0;
	//puts("\033[2J");
	//printf("\ndisplayTrackVolumes()");
	//Draw box
	cursor = 0;
	for(rdx=0; rdx<4; rdx++){
		g_lcdBuffer[cursor] = 0xff;
		g_lcdBuffer[cursor+1] = 0b10000001;
		cursor += LCD_WIDTH;
		g_lcdBuffer[cursor-1] = 0xff;
		g_lcdBuffer[cursor-2] = 0b10000001;
	}

	for(tdx=0; tdx<=TRACKCOUNT; tdx++){
		volbits = 0b11000000000000000000000000000000;
		//volbits = 0b00000000000000000000000000000011;

		//printf("\n\tTDX: %2d ", tdx);
		volume = (uint8_t)(g_song.volume[tdx]+15);
		//printf("\tvol:%2d = %2d ", g_song.volume[tdx], volume);
		volbits >>= volume;
		//volbits <<= volume;
		//Start point for each column
		cursor = (tdx * 5) + 2;
		//printf("\tcursor:%3u ", cursor);
		//Loop the height of the bar (4 rows)
		for(rdx=0; rdx<4; rdx++){
			chunk = (uint8_t)volbits;
			//printf("\n\t\trdx:%1u > %3u", rdx, chunk);
			//loop the width of the column
			for(wdx=0; wdx<5; wdx++){
				if((g_song.track == tdx) & (wdx == 2)){
					g_lcdBuffer[cursor+wdx] = (0b01010101 | chunk);
				}else{
					g_lcdBuffer[cursor+wdx] = chunk;
				}
			}
			//shift the next chunk
			volbits >>= 8;
			//Move cursor on a complete row
			cursor+=LCD_WIDTH;
		}
	}

	cursor = 0;
	for(tdx=0; tdx<4; tdx++){
		lcd_setxy(0,2+tdx);
		for(rdx=0; rdx<LCD_WIDTH; rdx++){
			lcd_writedata(g_lcdBuffer[cursor++]);
		}
	}

}

// delay in milli seconds
void delay_ms(uint16_t delay){
	uint32_t start = g_millis;
	while(g_millis - start < delay);
}

/*
 * Gathers input from the global input buffer.
 * Returns an input stuct
 */
InputRingBuffer_t gatherInput( void ){
	InputRingBuffer_t input;
	int8_t idx = 0;
	input.complete = 0x00000000;
	// We end up with a struct containing the debounced state of the buttons
	for(idx=0; idx<INPUTRINGLEN; idx++){
		input.complete = input.complete | g_input[idx].complete;
	}
	/*
	 * Now we have the buttons states OR'd together
	 * Any pressed button should be a zero.
	 * Active buttons are low so invert them to get a 1 for a pushed button
	 */
	input.complete = ~input.complete;
	return input;
}

/*
 * Increments the songlength
 */
void songLenInc(void){
	if(g_song.length < SONGLENMAX) g_song.length++;
}
void songLenDec(void){
	if(g_song.length > 0) g_song.length--;
}

void bpmUp(void){
	if(g_song.bpm < BPMMAX) g_song.bpm++;
	//Alter TMR2 period	
	bpmSetTmr();
}
void bpmDown(void){
	if(g_song.bpm > BPMMIN) g_song.bpm--;
	//Alter TMR2 period	
	bpmSetTmr();
}
void bpmSetTmr(void){
	/*
	 * System clock SYS_FREQ is so high now that tmr2 is too fast for our slow bpm in 16 bit mode.
	 * TMR is counting at 156Khz
	 * Our max count is 0xFFFF (65535) which gives us 2 quarter beats a second
	 * giving 30 bpm as our slowest beat
	 */
	float qps = (g_song.bpm * 4) / 60;
	//TMR2 is running at (Fosc) / 256
	uint32_t period = 156250 / qps;
	if(period > 0xffff) period = 0xffff;
	PR2 = (uint16_t)period;

}

void seq_toggleRepeat(void){
	g_systemState.repeat = ~g_systemState.repeat;
	if(g_systemState.repeat){
		//Light the repeat LED
		max6957_write(LED_REPEAT, 0x01);
	}else{
		//kill the repeat LED
		max6957_write(LED_REPEAT, 0x00);
	}
}
void toggleMenu(void){
	g_systemState.menu = ~g_systemState.menu;
	g_menuMode = 0;
	if(g_systemState.menu){
		//Light the menu LED
		max6957_write(LED_MENU, 0x01);
		lcd_clearRam();
		lcd_cursorxy(0,0);
		lcd_sendStr("Menu mode\n");
		lcd_cursorxy(0,1);
		lcd_sendStr("Use STEP buts.\n");
		lcd_cursorxy(0,2);
		lcd_sendStr("Confirm use ->\n");
		//xfprintf(&UART1PutChar, "\nEnter menu %d", g_menuMode);
	}else{
		//kill the menu LED
		max6957_write(LED_MENU, 0x00);
		lcd_clearRam();
		displayRunScreen();
	}
	
}

/*
 * Increments the pattern
 * Manages wrap
 */
void patternNext(void){
	if(g_song.pattern < g_song.length)
		g_song.pattern++;
	else
		g_song.pattern = 0;
}
/*
 * Decrements the pattern
 * Manages wrap
 */
void patternPrev(void){
	if(g_song.pattern > 0)
		g_song.pattern--;
	else
		g_song.pattern = g_song.length;
}
/*
 * Increments the track
 * Manages wrap
 */
void trackNext(void){
	if(g_song.track < g_trackCount) g_song.track++;
	else g_song.track = 0;
}
/*
 * Decrements the track
 * Manages wrap
 */
void trackPrev(void){
	if(g_song.track > 0) g_song.track--;
	else g_song.track = g_trackCount;
}

/*
 * Turns on the current step on the LEDs
 * LED is shown at high intensity compared
 * to current pattern which is shown as low intense
 * Step 0 is Max6957 P12
 * Use displayStepOff() to hide step LED
 */
void displayStepOn(int8_t step){
	step &= 0x0f; //We only have 16 LEDs so mask out anything higher
	//Now write the current step LED
	max6957_setSegCurrent(12 + step, 15);
	max6957_write(g_maxPortMap[step], 1);
}
/*
 * Turns off the current step on the LEDs
 * Returns the LED to it's current pattern status
 * Step 0 is Max6957 P12
 * Partner to displayStepOn()
 */
void displayStepOff(int8_t step){
	uint16_t stepmask = 0b1;
	uint16_t pattern = g_song.data[g_song.pattern][g_song.track];
	step &= 0x0f; //We only have 16 LEDs so mask out anything higher
	max6957_setSegCurrent(12 + step, LED_defaultIntensity);
	//determine if previous step should be lit or not
	if(pattern & (stepmask << step)){
		//Should be lit
		max6957_write(g_maxPortMap[step], 1);
	}else{
		//should be dark
		max6957_write(g_maxPortMap[step], 0);
	}
}

/*
 * Displays the current step on the LEDs
 * LED is shown at high intensity compared
 * to current pattern which is shown as low intense
 * Step 0 is Max6957 P12
 * Display is static
 */
void displayStepNextLEDs(int8_t step){
	uint16_t prevstep = 0;
	uint16_t stepmask = 0b1;
	uint16_t pattern = g_song.data[g_song.pattern][g_song.track];
	step &= 0x0f; //We only have 16 LEDs so mask out anything higher
	//First clear the previous step LED back to it's programmed pattern state
	prevstep = (step - 1) & 0x0f;
	max6957_setSegCurrent(12 + prevstep, LED_defaultIntensity);
	//determine if previous step should be lit or not
	if(pattern & (stepmask << prevstep)){
		//Should be lit
		max6957_write(g_maxPortMap[prevstep], 1);
	}else{
		//should be dark
		max6957_write(g_maxPortMap[prevstep], 0);
	}
	//Now write the current step LED
	max6957_setSegCurrent(12 + step, 15);
	max6957_write(g_maxPortMap[step], 1);
}

/*
 * Displays the programmed track for the current pattern
 * On the LEDS
 */
void displayPatternLEDs(void){
	uint16_t leds = g_song.data[g_song.pattern][g_song.track];
	int8_t idx = 0;
	//Can't bulk write as LEDs ports are not in sequence
	while(idx < 16){
		max6957_write(g_maxPortMap[idx], leds & 0b1);
		//shift the leds
		idx++;
		leds >>= 1;
	};
}

/*
 * Loads the samples.
 * Each samples index in g_sampData is it's track index in the patterns
 * Uses idx to load samples into tracks in the same order that you put the statments
 */
void setupSampleTracks(void){
	int8_t idx = 0;
	
	//Preload all channels with empty samples
	for(idx=0; idx<=CHANNELCOUNT; idx++){
		//Setup the channels to play silence (Sample 255)
		g_channel[idx].instrument = 255;
		//Putting dummy positions in to aid debug : They are ignored
		g_channel[idx].position = (idx + 1) * 10;
		//Default volume
		g_channel[idx].volume = 0;
	}

}


/*
General boot up stuff.
Sets IO
*/
void bootUp(void){
	SYSTEMConfig(SYS_FREQ, SYS_CFG_ALL);

	/*
		MX32 have SPI Clk pins hardwired not PPS. 
		USE SPI2 (clk on pin 26)
	*/
	PPSUnLock; //unlock Pin Re-map
	PPSOutput(2,RPB8,SDO2);
	PPSInput(3,SDI2,RPB13);
	PPSOutput(4,RPB9,OC3);
	PPSOutput(4,RPB14,U2TX);
	PPSLock; //lock Pin Re-map

	// enable multi-vector interrupts
  INTEnableSystemMultiVectoredInt();

	//Set all pins as digital
	ANSELA = 0x00000000;
	ANSELB = 0x00000000;
	
	LATA	= 0b1111111111111000;
	LATB	= 0b1111110111111111;
	TRISA = 0b0000000000000000; //1=input, 0=output
	TRISB = 0b0000100011111111; //1=input, 0=output
	CNENA	= 0; //No change ints
	CNENB	= 0; //No change ints
	CNPUB	= 0b0000000011111111; //Enable pullups on portb lower 8 bits

	//Start the core timer with a 1mS tick
	OpenCoreTimer(TICKS_mS);
	mConfigIntCoreTimer(CT_INT_ON | CT_INT_PRIOR_3);

	//Setup the PWM on timer 3
	OpenTimer3(T3_ON | T3_IDLE_STOP | T3_PS_1_1 | T3_SOURCE_INT , 255);
	OpenOC3( OC_ON | OC_IDLE_STOP | OC_TIMER_MODE16 | OC_TIMER3_SRC | OC_PWM_FAULT_PIN_DISABLE, PWM_duty_zero, PWM_duty_zero );

	//Set up Timer 1 : Fast : Used for mixing the samples and loading the PWM value
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_1, GetSystemClock() / 16000);
	// set up the timer interrupt
	ConfigIntTimer1(T1_INT_PRIOR_5);

	//Setup timer 2 : Runs the sequencer : Slow : around 1 - 4Hz
	OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_256, 0xffff);
	// set up the timer interrupt
	ConfigIntTimer2(T2_INT_PRIOR_3);

	//Set up Timer 4 : Middling : Used for getting input from the matrix
	OpenTimer4(T4_ON | T4_SOURCE_INT | T4_PS_1_64, (GetSystemClock()/64) / 300);
	// set up the timer interrupt
	ConfigIntTimer4(T1_INT_PRIOR_2);



	//SPI : Mode 3 (CLK Idle Low. Dat on rising edge)
	SpiChnOpen(
				SPI_CHANNEL2
			, SPI_OPEN_MSTEN
				| SPI_CONFIG_CKP_HIGH
				| SPI_OPEN_SMP_END 
				| SPI_OPEN_MODE8 
				| SPI_OPEN_SIDL
				, GetSystemClock()/4000000
		);

	

	//Assign UART1 TX to RP14 (RB14 - pin 25 on a 28 pin dip package)
	
	//UART1Init(103); //9600
	//UART1Init(34); //115200 = 114286  !NOT!
	UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY);
  UARTSetFifoMode(UART2, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
  UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
  //UARTSetDataRate(UART2, GetPeripheralClock(), 57600);
	UARTSetDataRate(UART2, GetPeripheralClock(), 115200);
  UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX) );

	


	lcd_Init();

	max6957setup();

	setupSampleTracks();
  loadSamples();

}

/*
 * Turns all step LEDs off
 */
void clearStepLED(){
	//bulk write
	max6957_write(MAX6957_ADDR_PG12_19, 0);
	max6957_write(MAX6957_ADDR_PG20_27, 0);	
}

//Turns the LCD BL on
void lcdBLOn(){
	max6957_write(LCD_BL, 1);
}
//Turns the LCD BL off
void lcdBLOff(){
	max6957_write(LCD_BL, 0);
}

void max6957setup(void){
	int8_t addr;
	//Config the Max

	max6957_config(0x00);

	max6957_config(MAX6957_SEGMENTCURRENT & MAX6957_NORMAL & MAX6957_TRANSOFF);
	//PDIP version has only ports 12 - 31
	
	//Set each port to zero : Use bulk port write
	for(addr = MAX6957_ADDR_PG12_19; addr <= MAX6957_ADDR_PG28_31; addr+=8){
		max6957_write(addr, 0x00);
	}
	
	//Set Port 15 as GPIO all others as LED
	max6957_write(MAX6957_ADDR_PCONF3, 0b01000000);
	for(addr = MAX6957_ADDR_PCONF4; addr <= MAX6957_ADDR_PCONF7; addr++){
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
		max6957_write(g_maxPortMap[addr], 0x01);
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
		max6957_write(g_maxPortMap[addr], 0x00);
		delay_ms(30);
	}
	
}


void seq_toggle(void){
	if(g_systemState.running == 1){
		seq_stop();
	}else{
		seq_run();
	}
}
void seq_run(void){
	WriteTimer2(0);
	INTClearFlag(INT_T2);
	EnableIntT2;
	g_systemState.running = 1;
	//Light the run LED
	max6957_write(LED_RUN, 0x01);
	}
void seq_stop(void){
	//Timer stays running, we just don't interrupt
	DisableIntT2;
	g_systemState.running = 0;
	max6957_write(LED_RUN, 0x00);
	}

void printBits(size_t const size, void const * const ptr){
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--){
        for (j=7;j>=0;j--){
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    }
    //puts("");
}