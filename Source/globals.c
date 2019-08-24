/* 
 * File:   globals.c
 * Author: Matt
 *
 * Created on 25 July 2012, 00:23
 * Declares global variables and structures
 */
#include "globals.h"

Channel_t g_channel[CHANNELCOUNT+1];
volatile Song_t g_song = {0, 0, 0, 15, 0, 120, 0, 50, .rate = {0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100,0x100}};
InputRingBuffer_t g_input[INPUTRINGLEN];

//Sample data : The raw samples themselves
uint16_t g_waveformLen[TRACKCOUNT+1];
const uint8_t *g_waveformData[TRACKCOUNT+1];
const char *g_waveformTitle[TRACKCOUNT+1];

uint8_t g_trackCount = 0;  //Actual number of samples defined. Set in setupSampleTracks()
int8_t g_songNumber = -1; // Used to store the index of the song to store eg. song_1.txt
uint16_t g_songExistanceBits = 0; // Used to store the existance state of each of the 16 song slots

volatile JammodeTrack_t g_jammode[TRACKCOUNT+1]; //Volume level and direction per track when in Jam mode

volatile int8_t g_reverb_buffer[REVERB_LENGTH];
volatile int16_t g_reverb_index = 0;

volatile uint8_t g_backlight = 0;
volatile uint32_t g_millis = 0;
volatile uint32_t Timer1 = 0;
volatile uint32_t Timer2 = 0;
volatile SystemState_t g_systemState;
volatile uint8_t g_inputstep = 0;

uint8_t g_lineBuf[16]; //Used for output to the LCD
char g_filename[20]; //Used to create and store the current filename for loading and saving

//char g_largebuffer[MAXFILESIZE];
Buffers_t g_buffer;


// delay in milli seconds
void delay_ms(uint16_t delay){
	uint32_t start = g_millis;
	while(g_millis - start < delay);
}

// Counting bits set, Brian Kernighan's way
uint8_t countBits(uint32_t v){
	uint8_t c = 0; // c accumulates the total bits set in v
	for (c = 0; v; c++){
		v &= v - 1; // clear the least significant bit set
	}
	return c;
}

/*
 * Returns the position (1 to 32) of 1st set (1) bit starting from LSB
 * 0 = No bit found.
 */
uint8_t firstBitPos(uint32_t value){
	uint8_t pos;
	for (pos = 1; value; pos++){
		if(value & 1) return pos;
		value >>= 1;
	}
	return 0;
}