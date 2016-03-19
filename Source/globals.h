/* 
 * File:   globals.h
 * Author: Matt
 *
 * Created on 24 July 2012, 23:54
 *
 * Defines global structure and variables
 */

#ifndef GLOBALS_H
#define	GLOBALS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

//Sample format is 8 bit signed but PWM output is unsigned
#define PWM_8bit_zero 128
#define Clip_max 127  //This for signed values
#define Clip_min -128 //This is for signed values

//PWM values
#define PWM_period 255
#define PWM_duty_zero 128

//No. of tracks : Each instrument(sample) has it's own track
#define TRACKCOUNT 15
//Pattern default LED intensity (0 <= X <= 15)
#define LED_defaultIntensity 1

//Channel struct array : These represent the polyphony
#define CHANNELCOUNT TRACKCOUNT

#define MAXFILESIZE 8192

#define PATTERNLENGTHMAX 15 //Maximum length of a pattern
#define SONGLENMAX 15 //Maximum length of any song
#define BPMMAX 255
#define BPMMIN ((GetSystemClock()/256)/0xffff)*(60/4) //Min bpm based on longest time period of 0xFFFF per quarter beat

//represents the state of the step button inputs
#define INPUTRINGLEN 6

//Limits to to the volume a track can play at
#define VOLUME_MIN -15
#define VOLUME_MAX 15

  
  
  
//Some VT100/ANSI escape codes
#define ESC \033[
#define CLS 2J
#define VREV ?5h
#define VNOR ?5l
#define ANSI(code) ANSI_AGAIN(033[ ## code)
#define ANSI_AGAIN(code) #code
  //#define ANSI(code) ANSI_AGAIN(code)
//#define ANSI_AGAIN(code) #\033[##code


#ifdef	__cplusplus
}
#endif

#endif	/* GLOBALS_H */



//Array used as a stack of structs describing the currently playing samples
typedef struct {
	uint8_t instrument;  //idx of sample to play
	int8_t volume; // Playback volume for this instrument
  uint16_t position; //current position in instrument sample
  int8_t direction; //Very wasteful... 
} Channel_t;

//Song data Lengths are 1 based, counters are 0 based
typedef struct {
	uint8_t pattern; 				//Current pattern (0 based)
	uint8_t track; 					//Current track (0 based)
	uint8_t step; 					//Current step (0 based)
	uint8_t patternLength;	//Number of steps in a pattern (0 based)
	uint8_t length; 				//Number of patterns in current song (0 based)
	uint8_t bpm;
	uint16_t data[SONGLENMAX+1][TRACKCOUNT+1];
  int8_t volume[TRACKCOUNT+1];  //Volume level per track (Entire song)
  int8_t voldir[TRACKCOUNT+1];  //Direction of change of track volume (Entire song)
} Song_t;

typedef union {
  char complete[MAXFILESIZE];
  union {
    char tophalf[MAXFILESIZE/2];
    char bothalf[MAXFILESIZE/2];
  };
} Buffers_t;

typedef union  {
    uint32_t complete;
    uint8_t bytes[4];
		struct {
			uint16_t steps;
			uint16_t buttons;
		};    
    struct {
			uint8_t stepsLo;
			uint8_t stepsHi;
			uint8_t buttonsLo;
			uint8_t buttonsHi;
		};
    struct {
			unsigned songlength:1;
			unsigned savesong:1;
			unsigned loadsong:1;
      unsigned jamming:1;
      unsigned :12;
			unsigned :8;					
			unsigned menu:1;
			unsigned repeat:1;
			unsigned run:1;
			unsigned down:1;
			unsigned right:1;
			unsigned up:1;
			unsigned left:1;
			unsigned sel:1;
  	};
} InputRingBuffer_t;


//Used to track current menu mode
typedef enum {
	MENU_DEFAULT,
	MENU_SONGLENGTH,
	MENU_SAVESONG,
	MENU_LOADSONG,
  MENU_JAMMODEON,
  MENU_JAMMODEOFF,
	MENU_OTHERTHING
} MenuMode_t;

typedef union  {
    uint8_t bytes[2];
    uint16_t complete;
		struct {
			unsigned :11;
      unsigned jamming:1; //Jamming mode. Volume cycles for each instrument
			unsigned menu:1; //We are in menu mode : The step buttons do other stuff now
			unsigned repeat:1; //Repeats the current pattern rather than incrementing
			unsigned trackchange:1; //track has been edited
			unsigned running:1; //sequencer is running
		};
} SystemState_t;

/*
 * Global variables
 */
extern volatile uint32_t g_millis; //Milli seconds counter
extern volatile uint32_t Timer1;		/* 1000Hz decrement timers used by FatFS but decremented by coretimer ISR */
extern volatile uint32_t Timer2;		/* 1000Hz decrement timers used by FatFS but decremented by coretimer ISR */
extern int test;
extern Channel_t g_channel[];
extern volatile Song_t g_song;
extern InputRingBuffer_t g_input[];

//Sample data : The raw samples themselves
extern uint32_t g_sampLen[];
//extern const int8_t *g_sampData[];
extern const int8_t *g_sampData[];
extern const char *g_sampTitle[];
extern uint8_t g_trackCount;
extern uint8_t g_songNumber;
extern const char *g_menuModeTitle[];
extern volatile SystemState_t g_systemState;
extern volatile MenuMode_t g_menuMode;
extern volatile uint8_t g_inputstep;
extern volatile uint8_t g_matrixrow;
extern uint8_t g_lineBuf[];
extern const uint8_t g_maxPortMap[];
extern void delay_ms(uint16_t);
extern char g_filename[];
//extern char g_largebuffer[];
extern Buffers_t g_buffer;

extern const char *testFile[];
