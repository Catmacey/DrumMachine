/*
	Save song in JSON format
	Loops over the current song and saves it out to the SD card in JSON format
	Returns a char to indicate success
	Note that lengths are stored internally as 0 based, so we add 1 to make them more human readable
	0 = success
*/

/****** I N C L U D E S **********************************************************/
#include <xc.h>
#include <plib.h>
#include "HardwareConfig.h"
#include <stdlib.h>
#include <stdint.h>
#include "globals.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "libs/xprintf.h"
#include "lcd/lcd.h"
#include "screen.h"
#include "json.h"
#include "song_saveload.h"

//File system vars
uint32_t AccSize;			/* Work register for fs command */
uint16_t AccFiles, AccDirs;
//FILINFO file_info;
#if _USE_LFN
char Lfname[512];
#endif
FATFS Fatfs;			/* File system object for each logical drive */
volatile BYTE rtcYear = 110, rtcMon = 10, rtcMday = 15, rtcHour, rtcMin, rtcSec;
FATFS *fs;				/* Pointer to file system object */
DIR dir;				/* Directory object */
FIL file_obj;		/* File objects */


/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

DWORD get_fattime (void){
	uint32_t tmr;
	//_DI();
	/* Pack date and time into a uint32_t variable */
	tmr =	  (((uint32_t)rtcYear - 80) << 25)
			| ((uint32_t)rtcMon << 21)
			| ((uint32_t)rtcMday << 16)
			| (uint16_t)(rtcHour << 11)
			| (uint16_t)(rtcMin << 5)
			| (uint16_t)(rtcSec >> 1);
	//_EI();
	return tmr;
}


static void put_rc (FRESULT rc){
	const char *str =
		"OK\0" "DISK\0" "INT\0" "NOT\0" "FILE\0" "PATH\0"
		"INV_NAME\0" "DENIED\0" "EXIST\0" "INV_OBJ\0" "WR_PROT\0"
		"INV_DRV\0" "NOT_EN\0" "NO_FSYS\0" "MKFS_AB\0" "TIMEOUT\0"
		"LOCKED\0" "NO_CORE\0" "TOO_MANY_OPEN\0";
	FRESULT i;

	for (i = 0; i != rc && *str; i++) {
		while (*str++) ;
	}
	xprintf("rc=%u %s\n", (UINT)rc, str);
}


//TODO Need to modify all these load/save routines to return meaningful error codes for display to user
//TODO Update save/load to include in/out points in the data


// Handles the interface for loading/saving
// Returns 1=success
int8_t song_loadOrSave(uint8_t load, InputRingBuffer_t *input){
	int8_t result = 0;
	uint8_t btnIdx = 0;
	lcd_clearBuffer();
	if(load){
		printf("Load song");
	}else{
		printf("Save song");
	}

	if(!g_systemState.existanceChecked){

		if(load && g_song.modified){
			boxOutWithTitle("Warning","Song modified");
			lcd_render();
			delay_ms(1000);
		}
		// Need to check the SD card to show all the existing songs
		result = song_checkExistance();
		if(!result){
			boxOutWithTitle("Error!","Can't access SD");
			lcd_render();
			// Get out of the menu
			delay_ms(1000);
			return 1;
		}
	}

	displayExistingSongs(load);
	lcd_render();
	printf(".");

	// handle input here
	if(input->steps && countBits((uint32_t)input->steps) == 1){
		btnIdx = firstBitPos((uint32_t)input->steps);

		printf("Load song:%x = %d", input->steps, btnIdx);

		//Load/save song file
		setFilename(btnIdx);
		boxOutWithTitle(load?"Loading...":"Saving...", g_filename);
		if(load){
			lcd_render();
			result = loadSong(btnIdx);
		}else{
			lcd_render();
			result = saveSong(btnIdx);
		}

		if(result){
			boxOutWithTitle(load?"Loaded!":"Saved!", g_filename);
			lcd_render();
			// Get out of the menu
			delay_ms(1000);
			return 1;
		}else{
			boxOutWithTitle("Failed!", g_filename);
		}
	}
	return 0;
}


void setFilename(uint8_t songIdx){
	xsprintf(g_filename, "/song%d.jsn", songIdx);
}

// Mounts the Filesytem.
// Any return value other than zero is an error
static uint8_t mountfs(){
	uint8_t fresult;
	printf("\nmountfs()");

	// Mount FS
	fresult = f_mount(&Fatfs, "", 1);
	if(fresult){
		printf("\nCan't mount SD. FRESULT:%d",fresult);
		put_rc(fresult);
	}

	printf(" Mounted:%u", fresult);
	return fresult;
}


int8_t song_checkExistance( void ){
	uint8_t dstatus;
	uint8_t idx = 0;
	FILINFO file_info;

	// I have an issue with SPI. Think I'm checking the wrong flag.
	// disk_initialize() - used in mountfs() gets stuck in a loop.
	// disabling and reenabling the SPI module sorts the problem
	// TODO: Fix this so I don't need to toggle the SPI module
	SpiChnEnable(SPI_CHANNEL2, 0);
//	printf("\nsong_checkExistance()");
	SpiChnEnable(SPI_CHANNEL2, 1);
	dstatus = mountfs();
	printf("\nmountfs():%u", dstatus);
	if(dstatus){
		// Failed
		return 0;
	}

	g_songExistanceBits = 0;

//	printf("\nChecking each song");

	for(idx=1; idx<=16; idx++){
		// Check each song in turn
		g_songExistanceBits <<= 1;
		setFilename(idx);
//		printf("\n%d \"%s\"", idx, g_filename);
		dstatus = f_stat(g_filename, &file_info);
//		printf(" [%u]", dstatus);
		if(!dstatus && file_info.fsize > 0 && file_info.fsize <= MAXFILESIZE){
//			printf(", %6d bytes", (uint32_t)file_info.fsize);
//			printf("1");
			g_songExistanceBits++;
//		}else{
//			printf("0");
		}
//		if(idx % 4 == 0){
//			printf(".");
//		}
//		dstatus = f_close(&file_obj);
//		printf("\nClose file: %u", dstatus);
	}
	// Indicate that we have checked the SD
	// This gets reset on every menu entry.
	// Should be sufficient to handle occasional card changes.
	g_systemState.existanceChecked = 1;
	return 1;
}

/*
 * Loads a file into memory
 * Better hope you have a enough free RAM...
 * Should probably allocate a buffer somewhere and enforce max size...
 * Looks for files names songX.jsn where X is 0-15 and provided by arg.songIdx
 * returns 1 to indicate success
*/
int8_t loadSong( uint8_t songIdx ){
	uint8_t dstatus;

	setFilename(songIdx);
	// xsprintf(g_filename, "/song%d.jsn", songIdx);
	printf("\nloadSong() file:%s", g_filename);
	dstatus = mountfs();
	if(dstatus){
		// Failed
		return 0;
	}

	// Open the file for reading
	dstatus = f_open(&file_obj, g_filename, FA_READ);
	if(dstatus){
		printf("\nCan't open file. FRESULT:%d",dstatus);
		put_rc(dstatus);
		return 0;
	}

	// More checks
	if(file_obj.fsize >= MAXFILESIZE){
		//xprintf("\nToo big:%6db", file_obj.fsize);
		printf("\nToo large:%6d bytes ", (uint32_t)file_obj.fsize);
		return 0;
	}
	//xprintf("\nSize %4dbytes", file_obj.fsize);
	printf("\nLoaded:%6d bytes ", (uint32_t)file_obj.fsize);

	f_gets(g_buffer.complete, file_obj.fsize, &file_obj);
	printf("\nFile read into buffer");
	f_close(&file_obj);
	printf("\nFile closed");

	dstatus = parseJSON(g_buffer.complete);
	if(dstatus){
		g_songNumber = (int8_t)songIdx;		
	}else{
		// Failed
		g_songNumber = -1;		
	}
	return dstatus;
}



/*
 * Saves the current song as a file in the SD card
 * Writes files named songX.jsn where X is 0-15 and provided by arg.songIdx
 * returns 1 to indicate success
*/
int8_t saveSong( uint8_t songIdx){
	uint8_t pattern, track, step, dstatus;

	setFilename(songIdx);

	printf("Save file:%s", g_filename);
	//xprintf("File:%s", g_filename);

	dstatus = mountfs();
	if(dstatus){
		// Failed
		return 0;
	}

	// Open the file for writing/creating
	dstatus = f_open(&file_obj, g_filename, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
	if(dstatus){
		printf("\nCan't open file. FRESULT:%d",dstatus);
		put_rc(dstatus);
		return 0;
	}
	
	//f_puts("//JSON Formatted song\r\r{\r\t\"saveformat\": 1,\r", &file_obj);
	f_puts("{\r\t\"saveformat\": 1,\r", &file_obj);
	f_printf(&file_obj, "\t\"bpm\": %d,\r", g_song.bpm);
	f_printf(&file_obj, "\t\"length\": %d,\r", g_song.length+1);
	f_printf(&file_obj, "\t\"patternlength\": %d,\r", g_song.patternLength+1);
	f_printf(&file_obj, "\t\"swing\": %d,\r", g_song.swing);

	// Save the track volumes
	f_printf(&file_obj, "\t\"trackvolumes\": [");
	for(track=0; track<=TRACKCOUNT; track++){
		if(track>0) f_puts(",", &file_obj);
		f_printf(&file_obj, "%d", g_song.volume[track]);
	}
	f_puts("],\r", &file_obj);

	// Save the track ratemods
	f_printf(&file_obj, "\t\"trackrates\": [");
	for(track=0; track<=TRACKCOUNT; track++){
		if(track>0) f_puts(",", &file_obj);
		f_printf(&file_obj, "%d", g_song.ratemod[track]);
	}
	f_puts("],\r", &file_obj);

	f_puts("\t\"patterns\": [\r", &file_obj);
	
	for(pattern=0; pattern<=g_song.length; pattern++){
		f_puts("\t\t[\r", &file_obj);
		//Loop each pattern
		for(track=0; track<=TRACKCOUNT; track++){
			//Each track
			f_puts("\t\t\t\"", &file_obj);
			for(step=0; step<=g_song.patternLength; step++){
				//Each step in the pattern
				if((g_song.data[pattern][track] >> step) & 0b1){
					//beat
					f_puts("^", &file_obj);
				}else{
					//no beat
					f_puts("-", &file_obj);
				}
			}//step
			f_puts("\"", &file_obj);
			if(track<TRACKCOUNT) f_puts(",", &file_obj);
			f_puts("\r", &file_obj);
		}//track
		f_puts("\t\t]", &file_obj);
		if(pattern<g_song.length) f_puts(",", &file_obj);
		f_puts("\r", &file_obj);
	}//pattern

	f_puts("\t]\r", &file_obj);
	f_puts("}\r", &file_obj);
	f_close(&file_obj);

	g_songNumber = (int8_t)songIdx;

	return 1;
}



