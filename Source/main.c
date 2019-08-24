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
/*
	#pragma config FPLLMUL = MUL_20 // PLL Multiply (now 80 MHz)
	#pragma config FPLLODIV = DIV_2 // Divide After PLL (now 40 MHz)
 */
//#pragma config FPLLIDIV = DIV_2 // Divide FRC before PLL (now 4 MHz)
//#pragma config FPLLMUL = MUL_24 // PLL Multiply (now 96 MHz)
//#pragma config FPLLODIV = DIV_2 // Divide After PLL (now 48 MHz)

//#pragma config FPLLIDIV = DIV_2 // Divide FRC before PLL (now 4 MHz)
//#pragma config FPLLMUL = MUL_15 // PLL Multiply (now 60 MHz)
//#pragma config FPLLODIV = DIV_1 // Divide After PLL (now 48 MHz)

#pragma config FPLLIDIV = DIV_2 // Divide FRC before PLL (now 4 MHz)
#pragma config FPLLMUL = MUL_16 // PLL Multiply (now 64 MHz)
#pragma config FPLLODIV = DIV_1 // Divide After PLL (now 48 MHz)


#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "libs/xprintf.h"
#include "fatfs/diskio.h"
#include "fatfs/ff.h"

//#include "splash.h"

//See inside HardwareConfig.h for pin defs
#include "HardwareConfig.h"
// Declare global variables
#include "globals.h"
// Low level SPI routines
#include "SPI.h"
// Loading and saving songs in JSON format to CD card
#include "song_saveload.h"
// Control over the running sequencer
#include "sequencer.h"
// Low level controls for displaying to the LCD
#include "lcd/lcd.h"
// Routines for drawing complex display elements to the LCD
#include "screen.h"
// Handles input/output whilst in menu mode
#include "menumode.h"
// Controls LED display
#include "leds.h"
//Sample data
#include "drumkit.h"

/*
 *	Function prototypes
 */
 
//void timer1Init(void);
//void timer2Init(void);
//void initpic(void);
void bootUp(void);


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
	int8_t result = 0;

	// Temp vars for tests
//	int8_t tdx = 0;
//	uint16_t adx = 0;
//	uint16_t bdx = 0;

	bootUp();

	mPORTBRead();

  _LATB14 = 1;

	//Set default output for prints
	xdev_out(lcd_putChar);

	setLEDstate(LCD_BL, ON);

	// Debug : Clear terminal screen
	puts("\033[2J");
	printf("Ready to go!");
  _LATB14 = 1;

	lcd_setCursor(0,0);
	lcd_putStr("Boot");

	g_systemState.complete = 0x0000;

	//NOTE set g_systemState.trackChange to trigger display of track/step on first run rather than
	//Current technique of setting pattern step and tracks to false values (15)

	//Stop the sequencer
	seq_stop();
	bpmSetTmr();

	lcd_setCursor(0,0);
	lcd_putStr("Drumkit Ready");
	lcd_render();
	
	idx = 0x80;
	// Flash the backlight
	while(idx--){
		setLEDintensity(LCD_BL, idx & 0x0f);
		delay_ms(5);
	}
	
	delay_ms(100);
	lcd_clearScreen();
	setLEDintensity(LCD_BL, 0x03);

	//preload g_input to avoid accidental triggering on reset
	for(idx=0; idx<INPUTRINGLEN; idx++){
		g_input[idx].complete = 0xffffffff;
	}
	previnput.complete = 0x00000000;

#ifdef __DEBUG
		//Debug : Add some dummy pattern
		g_song.data[0][0] = 0b0000010100000001;
		g_song.data[0][2] = 0b0001000000010000;
    //g_song.data[0][2] = 0b0101010101010101;
    //g_song.data[0][3] = 0b1111000011110000;
		g_systemState.repeat = 0b1;
		seq_run();
	#endif

	//Setup some volumes and directions for each track for jammode
	for(idx=0; idx <= TRACKCOUNT; idx++){
		g_jammode[idx].volume = 0;
		g_jammode[idx].voldir = 1;
	}


	// Messing with the default rates
	for(idx=0; idx <= CHANNELCOUNT; idx++){
		g_song.rate[idx] = RATE_DEFAULT;
		g_song.inpoint[idx] = 0;
		if(idx <= g_trackCount){
			g_song.outpoint[idx] = g_waveformLen[idx];
		}
	}//for


  _LATB14 = 0;

	//Enable Timer 1 interupt : Handles PWM loading and sample mixing
	EnableIntT1;
	//Enable Timer 4 interupt : Handles input
	EnableIntT4;

	// Test of volume drawing
	/*
	tdx = VOLUME_MIN;
	for(idx=0; idx<=TRACKCOUNT; idx++){
		g_song.volume[idx] = tdx;
		tdx+=2;
	}
	*/
	//lcd_clearScreen();

	//Main
	while(1){
//		setLEDstate(LCD_BL, ON);

//		mPORTBToggleBits(BIT_11);

		//Process the input ring buffer
		input = gatherInput();

		if(input.complete != previnput.complete){
			/*
			 * State is different to last time!
			 * Do something!
			 */

			// lcd_setCursor(0,0);
			// xprintf("S :%4x", input.buttons);

			//Important stuff first
			if(input.run) 		seq_toggle();
			if(input.menu) 		toggleMenu();
			if(input.repeat) 	seq_toggleRepeat();

			if(g_systemState.patternedit){
				// Do something...
			}else if(g_systemState.songload || g_systemState.songsave){
				g_systemState.menu = 1;
				if(input.menu){
					result = 1;
				}else{
					result = song_loadOrSave(g_systemState.songload, &input);
				}
				if(result){
					lcd_clearBuffer();
					g_systemState.songload = 0;
					g_systemState.songsave = 0;
					g_systemState.trackchange = 1;
					// Pretend to be in menu mode in order to come out of it.
					toggleMenu();
				}
			}else if(g_systemState.menu){
				// Catch menu after other modes that pretend to be a menu
				menu_main(input, previnput);
			}else{
				// Not menu mode
				if(input.up)			trackPrev();
				if(input.down)		trackNext();
				if(g_systemState.running){
					//Things we can do with running
					if(input.right) bpmUp();
					if(input.left)	bpmDown();
				}else{
					//Things to do whilst stopped
					if(input.right) patternNext();
					if(input.left)	patternPrev();
				}
				/*
				 * Dealing with the step buttons
				 */
				if(input.steps > 0 && input.steps != previnput.steps){
					//Step buttons set the beats
					g_song.data[g_song.pattern][g_song.track] ^= input.steps;
					g_song.modified = 1;
					if(!g_systemState.running){
						//play note if not running
						g_channel[0].position = (uint32_t)g_song.inpoint[g_song.track] << 8;  // Position is 24:8 fractional
						g_channel[0].instrument = g_song.track;
						g_channel[0].volume = g_song.volume[g_song.track];
						g_channel[0].rate = g_song.rate[g_song.track];
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


		//Display pattern/track data
		if(
				patternIdx != g_song.pattern
				|| trackIdx != g_song.track
				|| g_systemState.trackchange
		){
			//Display the current pattern/track on the LEDs
//			printf("Render pattern, %u, %u, %u", g_song.pattern, g_song.track, g_systemState.trackchange);
			patternIdx = g_song.pattern;
			trackIdx = g_song.track;
			led_displayPattern();
			if(!g_systemState.menu){
				displayPatternTrackCounters();
				displayPatternGraph(g_systemState.jamming?0:1);
			}
			g_systemState.trackchange = 0;
		}

		//Display BPM
		if(bpm != g_song.bpm && !g_systemState.menu){
			bpm = g_song.bpm;
			displayBPM();
		}

		if(step != g_song.step){
			step = g_song.step;
			//Step change
			if(!g_systemState.menu){
				displayStep();
				if(g_systemState.jamming){
					//Display the current track volumes
					displayTrackVolumes(49,19,-1);
				}
			}
			//Update the LEDs if running
			if(g_systemState.running){
				led_displayStepOn(step);
				// Run LED counts out the beat
				if((g_song.step & 0b00000011) == 0) setLEDintensity(RUN, 15);
				stepon = 1;
			}
		}
		if(stepon && (TMR2 > 3125)){
			//relies upon tmr2 being reset on each step
			//3125 is 50ms at TMR rate
			led_displayStepOff(step);
			if((g_song.step & 0b00000011) == 0) setLEDintensity(RUN, LED_INTENSITY_DEFAULT);
			stepon = 0;
		}
		//mPORTBToggleBits(BIT_14);

		// Render to screen : Might not send anything if no change in buffer
		lcd_render();
		// Maybe rather than just delaying here it would be better to look at a timer to decide how long to wait in order to keep a reasonably constant "frame rate"
		delay_ms(3);
	}//while

}//func


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
General boot up stuff.
Sets IO
*/
void bootUp(void){
	SYSTEMConfig(GetSystemClock(), SYS_CFG_ALL);

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
	TRISB = 0b0000000011111111; //1=input, 0=output
	CNENA	= 0; //No change ints
	CNENB	= 0; //No change ints
	CNPUB	= 0b0000000011111111; //Enable pullups on portb lower 8 bits

	//Start the core timer with a 1mS tick
	OpenCoreTimer(TICKS_mS);
	mConfigIntCoreTimer(CT_INT_ON | CT_INT_PRIOR_3);

	//Setup the PWM on timer 3
	OpenTimer3(T3_ON | T3_IDLE_STOP | T3_PS_1_1 | T3_SOURCE_INT , 1023);
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
//	OpenTimer4(T4_ON | T4_SOURCE_INT | T4_PS_1_64, (GetSystemClock()/64) / 3000);
	// set up the timer interrupt
//	ConfigIntTimer4(T1_INT_PRIOR_2);



	//SPI : Mode 3 (CLK Idle high. Dat on rising edge)
	SpiChnOpen(
				SPI_CHANNEL2
			, SPI_OPEN_MSTEN
				| SPI_CONFIG_CKP_HIGH
				| SPI_OPEN_SMP_END
				| SPI_OPEN_MODE8
				| SPI_OPEN_SIDL
				, GetSystemClock()/SPI_SPEED_FAST
		);

		//SPI : Mode 0 (CLK Idle low. Dat on rising edge)
//	SpiChnOpen(
//				SPI_CHANNEL2
//			, SPI_OPEN_MSTEN
//				| SPI_CONFIG_CKE_REV
//				| SPI_OPEN_SMP_END
//				| SPI_OPEN_MODE8
//				| SPI_OPEN_SIDL
//				, GetSystemClock()/SPI_SPEED_FAST
//		);

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


