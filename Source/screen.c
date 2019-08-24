/*
 * Routines that draw the various graphics to the screen buffer.
 * We never talk directly to the LCD here.
 * At least, that's the plan...
 */

/****** I N C L U D E S **********************************************************/
#include <xc.h>
#include <plib.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "HardwareConfig.h"
#include "globals.h"
#include "lcd/lcd.h"
#include "screen.h"
#include "libs/xprintf.h"
#include "song_saveload.h"
#include "drumkit.h"

void displaySampleWaveform( void );

void displayRunScreen(void){
	displayPatternTrackCounters();
	displayPatternGraph(g_systemState.jamming?0:1);
	displayBPM();
	displayStep();
}


// Displays the complete pattern, all 16 tracks all 16 steps
// Displays in 32 or 48 vertical px
// TODO: Rewrite to not directly access the screen buffer.
void displayPatternGraph(uint8_t tall){
	uint8_t tdx, sdx, width, rowheight;
	uint16_t pattern;

	if(tall > 0) {
		tall = 48;
		rowheight = 3;
	}else{
		tall = 32;
		rowheight = 2;
	}

	// Clear the area (lines 2 to 5-7) that we want to draw into
	lcd_fill_rect(0, 16, LCD_WIDTH, tall, 0);

	width = 6;
 
	// Draw line through the selected track
	lcd_draw_line(0, 17+(rowheight*g_song.track), LCD_WIDTH, 17+(rowheight*g_song.track), 2);

	// New style gfx lib
	for(tdx = 0; tdx <= g_trackCount; tdx++){
		// Loop each track
		// Get the pattern for the track
		pattern = g_song.data[g_song.pattern][tdx];

		// Do each step in the pattern
		for(sdx = 0; sdx <= g_song.patternLength; sdx++){
			if(pattern & 0b0000000000000001){
				// Draw a rectangle
				lcd_draw_rect(sdx*width, 16+(tdx*rowheight), width-1, rowheight, 1);
			}else{
				// Draw a dot
				lcd_set_pixel((sdx*width)+2, 17+(tdx*rowheight), 1);
			}
			pattern = pattern >> 1;
		}

	}
}



/*
 * Displays the current track volumes as 16 columns across the full width
 * Uses graphics commands rather than directly writing to the display buffer
 * Needs full width so no point passing left or width
 */
void displayTrackVolumes(uint8_t top, uint8_t height, int8_t selected){
	uint8_t tdx, colwidth, x, y ;

	colwidth = 5;
	// printf("\ndisplayTrackVolumes(top:%d, height:%d){colwidth:%d, zero:%d}", top, height, colwidth, zero);

	// Clear area
	// lcd_draw_rect(0,top,LCD_WIDTH-1,top+height-1,0);
	x = 0;
	for(tdx=0; tdx<=TRACKCOUNT; tdx++){
		// Volume is centered around 0
		if(g_systemState.jamming){
			// Display the dynamic jam mode volumes
			y = g_jammode[tdx].volume + VOLUME_MAX;
		}else{
			// Display the static song level volumes
			y = g_song.volume[tdx] + VOLUME_MAX;
		}
		drawVerticalSlider(x, top, colwidth, height, VOLUME_RANGE, y);
		if(selected == tdx){
			// Draw a box around the selected track
			lcd_draw_rect(x, top, colwidth, height, 1);
		}
		x += colwidth+1;
	}
}

void displayStep(void){
	lcd_setCursor(4,0);
	lcd_sendIcon(3);
	xprintf("%2u\n", g_song.step+1);
}

//Displays the current pattern and track at fixed locations on the LCD
void displayPatternTrackCounters(void){
	lcd_setCursor(0,0);
	lcd_sendIcon(4);
	xprintf("%2u\n", g_song.pattern+1); //+1 to make it human
	lcd_setCursor(0,1);
	lcd_sendIcon(0);
	xprintf("%2u\n", (int8_t)g_song.track+1);  //+1 to make it human
	lcd_putStr(g_waveformTitle[g_song.track]);
}

//Displays the current BPM at a fixed location on the LCD
void displayBPM(void){
	lcd_setCursor(8,0);
	lcd_sendIcon(1);
	xprintf("%3u\n", g_song.bpm);
	// Show swing
	lcd_setCursorXRow(78, 0);
	xprintf("%2u%%\n", g_song.swing);
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

/*
 * Boxes out a single line of text
 * Handy as a state change alert
 * Really simple, doesn't wrap or crop
 */
void boxOut(char *text){
	lcd_fill_rect( 4, 19, LCD_WIDTH - 8, 17, 0);
	lcd_draw_rect( 5, 20, LCD_WIDTH - 10, 15, 1);
	lcd_setCursor(2,3);
	lcd_putStr(text);
}

/*
 * Boxes out a single line of text with an inverse title
 * Handy as a state change alert
 * Really simple, doesn't wrap or crop
 */
void boxOutWithTitle(char *title, char *text){
	// Draw title and text
	lcd_fill_rect( 3, 11, LCD_WIDTH - 6, 25, 0);
	lcd_draw_rect( 4, 12, LCD_WIDTH - 8, 23, 1);
	lcd_setCursorXRow(8,2);
	lcd_setTextMode( BOLD );
	lcd_putStr(title);
	lcd_setTextMode( NORMAL );
	lcd_setCursorXRow(8,3);
	lcd_putStr(text);
}

/*
 * Displays the current track volumes as 16 columns across the full width
 * Uses graphics commands rather than directly writing to the display buffer
 * Needs full width so no point passing left or width
 */
void displayTrackRates(uint8_t top, uint8_t height, int8_t selected){
	uint8_t tdx, colwidth, x, y;
	colwidth = 5;
	x = 0;
	for(tdx=0; tdx<=TRACKCOUNT; tdx++){
		y = g_song.ratemod[tdx] + RATEMOD_MAX;
		drawVerticalSlider(x, top, colwidth, height, RATEMOD_RANGE, y);
		if(selected == tdx){
			// Draw a box around the selected track
			lcd_draw_rect(x, top, colwidth, height, 1);
		}
		x += colwidth+1;
	}
}

/*
 * Displays the sample waveform and the start/end cursors
 */
void displaySampleCropping(uint8_t cursor, uint8_t stepsize){
	lcd_clearBuffer();
	lcd_setCursor(0,0);
	lcd_sendIcon(0);
	xprintf("%2d ", (int8_t)g_song.track+1);  //+1 to make it human
	lcd_putStr(g_waveformTitle[g_song.track]);
	lcd_setCursor(0,1);
	xprintf("Len:%6d", g_waveformLen[g_song.track]);
	
	lcd_setCursor(0,2);
	if(cursor){
		lcd_setTextMode(INVERSE);
	}
	xprintf("I:%5d", g_song.inpoint[g_song.track]);
	lcd_setTextMode(NORMAL);
	
	lcd_setCursor(8,2);
	if(!cursor){
		lcd_setTextMode(INVERSE);
	}
	xprintf("O:%5d", g_song.outpoint[g_song.track]);
	lcd_setTextMode(NORMAL);
	
	drawInOutPoints(0, 24, LCD_WIDTH, 8, g_waveformLen[g_song.track], g_song.inpoint[g_song.track], g_song.outpoint[g_song.track]);
	displaySampleWaveform();
	lcd_setCursor(0,0);
	xprintf("S :%1d", stepsize);
}


/*
 * Displays a lo-rez version of the current sample
 * Limited to the width of the LCD
 * designed to be fast not particularily accurate.
 * Bucketsize * no.buckets will always be <= total sample length
 * This means we might miss some samples at the end, but who cares...
 */
void displaySampleWaveform( void ){
	uint8_t bucketsize = g_waveformLen[g_song.track] / LCD_WIDTH;
	uint32_t accum;
	uint8_t idx, jdx, ypos;
	uint16_t pos = 0;
	lcd_fill_rect(0, 32, LCD_WIDTH, 32, 0);

	for(idx=0; idx < LCD_WIDTH; idx++){
		accum = 0;
		for(jdx=0; jdx < bucketsize; jdx++){
			// Add the absolute value 0 <= X <= 511
			accum += abs(from8bit_table[g_waveformData[g_song.track][pos++]]);
		}
		// average
		accum /= bucketsize;
		// Scale from 9bit to 5bit
		accum >>= 4;

//		accum = idx;
		if(accum > 32){
			accum = 32;
		}

		ypos = 32 + ((32 - (uint8_t)accum) / 2);
		lcd_draw_line(idx, ypos, idx, ypos+(uint8_t)accum, 1);
	}
}


/*
 * Displays the graph of track volumes and playback rates
 * Takes up entire screen : Can br displayed in run mode but cannot mix with other screens
 *
 */
void displayTuningGraph(uint8_t state){
	lcd_clearBuffer();
	lcd_setCursor(0,0);
	//lcd_setTextMode( BOLD );
	// lcd_putStr("Tuning mode");
	//lcd_setTextMode( NORMAL );
	// TODO: Draw "box" around current state (0 = volume, 1 = rate) in tuning mode
	lcd_setCursor(0,0);
	lcd_sendIcon(0);
	xprintf("%2d ", (int8_t)g_song.track+1);  //+1 to make it human
	lcd_putStr(g_waveformTitle[g_song.track]);
	if(state){
		lcd_setTextMode(INVERSE);
	}
	lcd_setCursor(0,1);
	xprintf("Vol%3d", g_song.volume[g_song.track]);
	lcd_setTextMode(NORMAL);

	if(!state){
		lcd_setTextMode(INVERSE);
	}
	lcd_setCursor(8,1);
	xprintf("Rate%3d", g_song.ratemod[g_song.track]);
	lcd_setTextMode(NORMAL);

	displayTrackVolumes(16 ,25, (state)?(int8_t)g_song.track:-1);
	displayTrackRates(43, 25, (!state)?(int8_t)g_song.track:-1);
}

// Displays what there is in g_songExistance. Hope you updated it recently
void displayExistingSongs(uint8_t load){
	uint8_t idx, cdx, rdx;
	uint16_t mask = 0b1000000000000000;
	printf("\ndisplayExistingSongs()");

	// Check to see what songs are on the SD
	lcd_clearBuffer();
	lcd_setTextMode( BOLD );
	lcd_putStr(load?"Load song":"Save song");
	lcd_setTextMode( NORMAL );
	lcd_setCursor(0,1);
	lcd_putStr("Select a slot");

	lcd_setCursor(0,7);
	if(g_songNumber > 0){
		xprintf("Current song: %u", (uint8_t)g_songNumber);
	}else{
		xprintf("No song loaded");
	}

	lcd_setCursor(0,2);
	lcd_putStr("Existing songs");
	

	printf("\nGot some songs: %x", g_songExistanceBits);
	//lcd_setTextMode( BOLD );
	// Draw 16 boxes showing the existance state of each song
	idx = 0;
	for(rdx=0; rdx<4; rdx++) {
		for(cdx=0; cdx<4; cdx++){
			if(g_songExistanceBits & mask){
				// Exists
				lcd_fill_rect((cdx*24), 24+(rdx*8), 23, 7, 1);
			}else{
				// Doesn't
				lcd_draw_rect((cdx*24), 24+(rdx*8), 23, 7, 1);
			}
			mask >>= 1;
			idx++;
		}
	}
}


/*
 * Draws a single vertical slider at the given position
 * Doesn't check that the coords you give are on the screen
 */

void drawVerticalSlider(uint8_t left, uint8_t top, uint8_t width, uint8_t height, uint8_t max, uint8_t value){
	float scale = (float)value/max;

	uint8_t vpos, voff;
	// Sanity
	if(height < 8 || width < 3) return;

	voff = floorf((scale*(height-1))+0.5);
	vpos = height-voff-1;

	// printf("\ndrawVerticalSlider(left:%d, top:%d, w:%d, h:%d, max:%d, v:%d){scale:%li.%li, voff:%d, vpos:%d}", left, top, width, height, max, value, lWhole, ulPart, voff, vpos);

	// Clear the rectangle
	lcd_fill_rect(left, top, width, height, 0);

	// Draw a vertical line in the center
	if (width % 2 == 0){
		// Width is an even number : Draw a double width line
		lcd_draw_line(left+(width/2)-1,top,left+(width/2)-1,top+height-1,1);
	}
	lcd_draw_line(left+(width/2),top,left+(width/2),top+height-1,1);

	// Draw the scale line at the correct vertical position
	lcd_draw_line(left,vpos+top,left+width-1,vpos+top,1);
}

/*
 * Draws a single horizontal slider at the given position
 * Doesn't check that the coords you give are on the screen
 */
void drawHorizontalSlider(uint8_t left, uint8_t top, uint8_t width, uint8_t height, uint16_t max, uint16_t value){
	float scale = (float)value/max;
	uint8_t halfheight = (height/2);
	uint8_t hpos, hoff;

	// Sanity
	if(height < 3 || width < 8) return;

	width -= 1;

	hoff = floorf((scale*(width))+0.5);
	hpos = width-hoff-1;

	// Clear the rectangle
	lcd_fill_rect(left, top, width, height, 0);

	// Draw a vertical line in the center
	if (height % 2 == 0){
		// Width is an even number : Draw a double width line
		lcd_draw_line(left, top+halfheight-1, left+width, top+halfheight-1, 1);
	}
	lcd_draw_line(left, top+halfheight, left+width, top+halfheight, 1);

	// Draw the scale line at the correct vertical position
	lcd_draw_line(left+hpos, top, left+hpos, top+height-1, 1);
}

/*
 * Draws a single horizontal in/out slider at the given position
 * Doesn't check that the coords you give are on the screen
 */
void drawInOutPoints(uint8_t left, uint8_t top, uint8_t width, uint8_t height, uint16_t max, uint16_t inpoint, uint16_t outpoint){
	float scale = (float)inpoint/max;
	uint8_t halfheight = top + (height/2);
	uint8_t hpos, bot, right;

	// Sanity
	if(height < 3 || width < 8) return;

	width -= 1;
	right = left + width - 1;

//	hoff = floorf((scale*(width))+0.5);
//	hpos = width-hoff-1;

	// Clear the rectangle
	lcd_fill_rect(left, top, width, height, 0);

	// Draw a vertical line in the center
	if (height % 2 == 0){
		// Width is an even number : Draw a double width line
		lcd_draw_line(left, halfheight-1, right, halfheight-1, 1);
	}
	lcd_draw_line(left, halfheight, right, halfheight, 1);

	bot = height + top - 1;
	
	// Draw inpoint
	hpos = left+((uint8_t)floorf((scale*(width))+0.5));
	lcd_draw_line(hpos, top, hpos, bot, 1);
	if(hpos > 0){
		lcd_draw_line(hpos-1, top, hpos, top, 1);
//		lcd_draw_line(hpos-1, bot, hpos, bot, 1);
	}

	// Draw outpoint
	scale = (float)outpoint/max;
	hpos = left+((uint8_t)floorf((scale*(width))+0.5));
	lcd_draw_line(hpos, top, hpos, bot, 1);
//	lcd_draw_line(hpos-1, top, hpos, top, 1);
	lcd_draw_line(hpos+1, bot, hpos, bot, 1);
}