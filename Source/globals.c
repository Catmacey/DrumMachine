/* 
 * File:   globals.c
 * Author: Matt
 *
 * Created on 25 July 2012, 00:23
 * Declares global variables and structures
 */
#include "globals.h"
#include "maxim6957.h"

Channel_t g_channel[CHANNELCOUNT+1];
volatile Song_t g_song = {0, 0, 0, 15, 0, 120};
InputRingBuffer_t g_input[INPUTRINGLEN];

//Sample data : The raw samples themselves
uint32_t g_sampLen[TRACKCOUNT+1];
const signed char *g_sampData[TRACKCOUNT+1];
const char *g_sampTitle[TRACKCOUNT+1];
unsigned char g_trackCount = 0;  //Actual number of samples defined. Set in setupSampleTracks()
unsigned char g_songNumber = 0; // Used to store the index of the song to store eg. song_1.txt

const char *g_menuModeTitle[] = {"Default","Song len","Save","Load","Jam mode ON","Jam mode OFF"};

const char *testFile[] = {"{\"firstobj\":\"string\",\"secondobj\":1234}"};


volatile uint32_t g_millis = 0;
volatile uint32_t Timer1 = 0;
volatile uint32_t Timer2 = 0;

volatile SystemState_t g_systemState;
volatile MenuMode_t g_menuMode;

volatile unsigned char g_inputstep = 0;

volatile unsigned char g_matrixrow = 0; //used by matrix ISR keep track of current row

unsigned char g_lineBuf[14]; //Used for output to the LCD
char g_filename[20]; //Used to create and store the current filename for loading and saving

//char g_largebuffer[MAXFILESIZE];
Buffers_t g_buffer;


/*
 Array of MAX6957 port addresses for the step LEDs. 
 Due to circuit layout constraints the The LEDs are not connected
 sequentially so we need to map from step index to MAX port address
 Note: The last four are run,repeat,menu and BL
 */
const unsigned char g_maxPortMap[] = {
		MAX6957_ADDR_P12 // Step1
	,	MAX6957_ADDR_P13 // 2
	,	MAX6957_ADDR_P14 // 3
	,	MAX6957_ADDR_P21 // 4
	,	MAX6957_ADDR_P22 // 5
	,	MAX6957_ADDR_P27 // 6
	,	MAX6957_ADDR_P29 // 7
	,	MAX6957_ADDR_P31 // 8
	,	MAX6957_ADDR_P16 // 9
	,	MAX6957_ADDR_P17 // 10
	,	MAX6957_ADDR_P18 // 11
	,	MAX6957_ADDR_P20 // 12
	,	MAX6957_ADDR_P19 // 13
	,	MAX6957_ADDR_P23 // 14
	,	MAX6957_ADDR_P24 // 15
	,	MAX6957_ADDR_P25 // Step 16
	,	MAX6957_ADDR_P26 // Run
	,	MAX6957_ADDR_P28 // Repeat
	,	MAX6957_ADDR_P30 // Menu
	,	MAX6957_ADDR_P15 // LCD BL
};


