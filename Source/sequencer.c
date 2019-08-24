/*
 * Routines that manage the playing of the song
 */

/****** I N C L U D E S **********************************************************/
#include <xc.h>
#include <plib.h>
#include "HardwareConfig.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "globals.h"
#include "lcd/lcd.h"
#include "screen.h"
#include "sequencer.h"
#include "libs/xprintf.h"
#include "leds.h"


/*
 * Increments the songlength
 */
void songLenInc(void){
	if(g_song.length < SONGLENMAX){
		g_song.length++;
		g_song.modified = 1;
	}
}
void songLenDec(void){
	if(g_song.length > 0){
		g_song.length--;
		g_song.modified = 1;
	}
}

void inpointModify(uint8_t track, int8_t value){
	// Inpoint is always less than outpoint
	int32_t point;
	if(track>TRACKCOUNT) return;
	if(value == 0) return;
	point = g_song.inpoint[track] + value;
	if(value > 0){
		// going up
		if(point >= g_song.outpoint[track]){
			point = g_song.outpoint[track] - 1;
		}
	}else{
		// going down
		if(point < 0){
			point = 0;
		}
	}
	g_song.inpoint[track] = (uint16_t)point;
	g_song.modified = 1;
}

void outpointModify(uint8_t track, int8_t value){
	// Outpoint is always more than inpoint
	int32_t point;
	if(track>TRACKCOUNT) return;
	if(value == 0) return;
	point = g_song.outpoint[track] + value;
	if(value > 0){
		// going up
		if(point > g_waveformLen[track]){
			point = g_waveformLen[track];
		}
	}else{
		// going down
		if(point <= g_song.inpoint[track]){
			point = g_song.inpoint[track] + 1;
		}
	}
	g_song.outpoint[track] = (uint16_t)point;
	g_song.modified = 1;
}

void swingSet(uint8_t value){
	if(value > SWING_MAX){
		value = SWING_MAX;
	}else if(value < SWING_MIN){
		value = SWING_MIN;
	}
	g_song.swing = value;
	g_song.modified = 1;
	// Calculate the new odd/even timer2 period
	bpmSetTmr();
}

void swingModify(int8_t value){
	int8_t newswing = g_song.swing + value;
	swingSet(newswing);
}

void rateModify(uint8_t track, int8_t value){
	int8_t newrate;
	if(track>TRACKCOUNT) return;
	newrate = g_song.ratemod[track] + value;
	if(newrate > RATEMOD_MAX){
		newrate = RATEMOD_MAX;
	}else if(newrate < RATEMOD_MIN){
		newrate = RATEMOD_MIN;
	}
	g_song.ratemod[track] = newrate;
	g_song.rate[track] = calcRate(g_song.ratemod[track]);
	g_song.modified = 1;
}
void rateUp(uint8_t track){
	if(track>TRACKCOUNT) return;
	if(g_song.ratemod[track] < RATEMOD_MAX){
		g_song.ratemod[track]++;
		g_song.rate[track] = calcRate(g_song.ratemod[track]);
		g_song.modified = 1;
	}
}
void rateDown(uint8_t track){
	if(track>TRACKCOUNT) return;
	if(g_song.ratemod[track] > RATEMOD_MIN){
		g_song.ratemod[track]--;
		g_song.rate[track] = calcRate(g_song.ratemod[track]);
		g_song.modified = 1;
	}
}

void volumeModify(uint8_t track, int8_t value){
	int8_t newvol;
	if(track>TRACKCOUNT) return;
	newvol = g_song.volume[track] + value;
	if(newvol > VOLUME_MAX){
		newvol = VOLUME_MAX;
	}else if(newvol < VOLUME_MIN){
		newvol = VOLUME_MIN;
	}
	g_song.volume[track] = newvol;
	g_song.modified = 1;
}
void volumeUp(uint8_t track){
	if(track>TRACKCOUNT) return;
	if(g_song.volume[track] < VOLUME_MAX){
		g_song.volume[track]++;
		g_song.modified = 1;
	}
}
void volumeDown(uint8_t track){
	if(track>TRACKCOUNT) return;
	if(g_song.volume[track] > VOLUME_MIN){
		g_song.volume[track]--;
		g_song.modified = 1;
	}
}

void bpmSet(uint8_t value){
	if(value > BPMMAX){
		value = BPMMAX;
	}else if(value < BPMMIN){
		value = BPMMIN;
	}
	g_song.bpm = value;
	g_song.modified = 1;
	//Alter TMR2 period
	bpmSetTmr();
}

void bpmModify(int8_t value){
	uint8_t newbpm = g_song.bpm;
	newbpm += value;
	bpmSet(newbpm);
}

void bpmUp(void){
	if(g_song.bpm < BPMMAX){
		g_song.bpm++;
		g_song.modified = 1;
		//Alter TMR2 period
		bpmSetTmr();
	}
}
void bpmDown(void){
	if(g_song.bpm > BPMMIN){
		g_song.bpm--;
		g_song.modified = 1;
		//Alter TMR2 period
		bpmSetTmr();
	}
}
void bpmSetTmr(void){
	/*
	 * System clock SYS_FREQ is so high now that tmr2 is too fast for our slow bpm in 16 bit mode.
	 * TMR is counting at 156Khz (187.5Khz @ 48Mhz)
	 * Our max count is 0xFFFF (65535) which gives us 2 quarter beats a second
	 * giving 30 bpm as our slowest beat @FOSC 40MHz
	 */
	float qps = (g_song.bpm * 4) / 60;
	//TMR2 is running at (Fosc) / 256
	uint32_t period_odd = (GetSystemClock()/256) / qps;
	uint32_t period_even = period_odd;
	uint16_t diff = 0;

	// Should we apply some swing?
	// Note no swing = 50
	if(g_song.swing > SWING_MIN){
		diff = (period_odd / 100) * (g_song.swing-SWING_MIN);
		// Odd
		period_odd += diff;
		if(period_odd > 0xffff) period_odd = 0xffff;
		// Even
		period_even -= diff;
		if(period_even > 0xffff) period_even = 0xffff;
	}

	g_song.swing_timer_odd = (uint16_t)period_odd;
	g_song.swing_timer_even = (uint16_t)period_even;
}

void seq_toggleRepeat(void){
	g_systemState.repeat = ~g_systemState.repeat;
	if(g_systemState.repeat){
		//Light the repeat LED
		setLEDstate(REPEAT, 0x01);
	}else{
		//kill the repeat LED
		setLEDstate(REPEAT, 0x00);
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

// Toggle start/stop of sequencer
void seq_toggle(void){
	if(g_systemState.running == 1){
		seq_stop();
	}else{
		seq_run();
	}
}
// Stop sequencer
void seq_run(void){
	WriteTimer2(0);
	INTClearFlag(INT_T2);
	EnableIntT2;
	g_systemState.running = 1;
	//Light the run LED
	setLEDstate(RUN, 0x01);
	}
// Start sequencer
void seq_stop(void){
	//Timer stays running, we just don't interrupt
	DisableIntT2;
	g_systemState.running = 0;
	setLEDstate(RUN, 0x00);
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
 * Calculates the actual rate from the linear rate mod value
 * Rate scales from 128(halfspeed) to 512(doublespeed) and is centered at 256 (0x100)
 * whilst ratemod ranges from -32 to +32 and is centered at 0
 */
int16_t calcRate(int8_t ratemod){
//	float value = RATE_DEFAULT;
	uint16_t value = RATE_DEFAULT;
	if(ratemod == 0) return RATE_DEFAULT;
	if(ratemod > 0){
		while(ratemod--){
//			value *= 1.0625;
//			value *= 1.03125;
			value += 8;
		}
	}else{
		while(ratemod++ < 0){
//			value *= 0.9375;
			// value *= 0.96875;
			value -= 4;
		}
	}
	// printf("calcRate(%d){%d}", ratemod, (uint16_t)value);
	return (uint16_t)value;
}