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
#define PWM_8bit_zero 512
#define Clip_max 511  //This for signed values
#define Clip_min -511 //This is for signed values

//PWM values
#define PWM_period 1023
#define PWM_duty_zero 512

//No. of tracks : Each instrument(sample) has it's own track
#define TRACKCOUNT 15

//Channel struct array : These represent the polyphony
#define CHANNELCOUNT TRACKCOUNT

#define MAXFILESIZE 8192

#define PATTERNLENGTHMAX 15 //Maximum length of a pattern
#define SONGLENMAX 15 //Maximum length of any song
#define BPMMAX 255
#define BPMMIN ((GetSystemClock()/256)/0xffff)*(60/4) //Min bpm based on longest time period of 0xFFFF per quarter beat

//represents the state of the step button inputs
#define INPUTRINGLEN 10

//Limits to to the volume a track can play at
#define VOLUME_MIN -15
#define VOLUME_MAX 15
#define VOLUME_RANGE 30

// Limits to playback rate
#define RATE_MIN 0x20 // 8 times slower
#define RATE_STEP 0x08
#define RATE_STEPS 0xFF
#define RATE_DEFAULT 0x100 // 1:1 playback
#define RATE_MAX RATE_MIN + (RATE_STEP * RATE_STEPS) // 8 times faster (approx)
#define RATE_RANGE RATE_MAX - RATE_MIN // Range is 2048 in 256 steps
#define SWING_MIN 50
#define SWING_MAX 90

// Limits to the interface that modifys the rate
#define RATEMOD_MIN -32
#define RATEMOD_MAX 32
#define RATEMOD_RANGE 64

//TODO (Maybe) Balance the rate around a centerpoint. Eg if I use 0x100 as 1:1 playback then should rate be a int8_t with zero at 1:1

// Reverb buffer length : For now this is a distinct buffer, better to integrate into Buffers_t
#define REVERB_LENGTH 2048

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
  uint32_t position; //current position in instrument sample : This is used as a fixed point number top 24bit are the integer part, bottom 8bit is fraction
  uint16_t rate; // This is the rate at which the sample should be played. Default 1:1 rate is 0xff higher rate results in sample being played faster
  int16_t prevSample; // Holds the previous sample for linear interpolation
  // int8_t direction; //Very wasteful...
} Channel_t;

//Song data Lengths are 1 based, counters are 0 based
typedef struct {
	uint8_t pattern; 				// Current pattern (0 based)
	uint8_t track; 					// Current track (0 based)
	uint8_t step; 					// Current step (0 based)
	uint8_t patternLength;	// Number of steps in a pattern (0 based)
	uint8_t length; 				// Number of patterns in current song (0 based)
	uint8_t bpm;
	uint8_t modified;				// Set if you modify the song after loading. Used as a warning when saving.
	uint8_t swing;				// amount of swing (even beat offset)
  uint16_t swing_timer_odd;  // Period for odd beats
  uint16_t swing_timer_even;  // Period for even beats
  uint16_t data[SONGLENMAX+1][TRACKCOUNT+1];
  int8_t volume[TRACKCOUNT+1];  // Volume level per track (Entire song)
  uint16_t rate[TRACKCOUNT+1];  // The actual playback speed per track (Entire song) Uses fixed pos. bits 0-7 are fractional : This is calculated
  int8_t ratemod[TRACKCOUNT+1];  // Interface to modify the rate : This is what we store in the song file
  uint16_t inpoint[TRACKCOUNT+1]; // Start position for cropped samples 0 < X < waveform length
  uint16_t outpoint[TRACKCOUNT+1]; // End position for cropped samples 0 < X < waveform length
} Song_t;

// Jammode structure
typedef struct {
  int8_t volume;  //Volume level per track (Entire song)
  int8_t voldir;  //Direction of change of track volume (Entire song)
} JammodeTrack_t;

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


typedef union  {
    uint8_t bytes[2];
    uint16_t complete;
		struct {
			unsigned :6;
      unsigned patternedit:1; // Display for editing patterns
			unsigned songload:1; // Display for loading songs
			unsigned songsave:1; // Display for saving songs
			unsigned existanceChecked:1; // Have recently checked the SD for song existance (reset every menu entry)
      unsigned reverb:1; //Reverb output
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
extern volatile uint8_t g_backlight; // Backlight value. Stored globally for menu access
extern volatile uint32_t g_millis; //Milli seconds counter
extern volatile uint32_t Timer1;		/* 1000Hz decrement timers used by FatFS but decremented by coretimer ISR */
extern volatile uint32_t Timer2;		/* 1000Hz decrement timers used by FatFS but decremented by coretimer ISR */
extern int test;
extern Channel_t g_channel[];
extern volatile Song_t g_song;
extern InputRingBuffer_t g_input[];

//Sample data : The raw samples themselves
extern uint16_t g_waveformLen[];
extern const uint8_t *g_waveformData[];
extern const char *g_waveformTitle[];
extern uint8_t g_trackCount;

extern int8_t g_songNumber;
extern uint16_t g_songExistanceBits;

extern const char *g_menuModeTitle[];
extern volatile SystemState_t g_systemState;
// extern volatile MenuMode_t g_menuMode;
extern volatile uint8_t g_inputstep;
extern uint8_t g_lineBuf[];
extern char g_filename[];
//extern char g_largebuffer[];
extern Buffers_t g_buffer;
extern volatile JammodeTrack_t g_jammode[];

extern volatile int8_t g_reverb_buffer[];
extern volatile int16_t g_reverb_index;

// Functions
void delay_ms(uint16_t);
uint8_t countBits(uint32_t v);
uint8_t firstBitPos(uint32_t value);
