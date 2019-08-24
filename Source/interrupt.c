#include <xc.h>
#include <plib.h>
#include "HardwareConfig.h"
#include "globals.h"
#include "drumkit.h"

//_T1Interrupt() is the T1 interrupt service routine (ISR). Runs at sample freq
void __ISR(_TIMER_1_VECTOR, IPL5SOFT) Timer1Handler(void){
//	mPORTBSetBits(BIT_11);
	//channel freqs
	int16_t output = 0; // Summed output of all channels
	int16_t sample1 = 0;
	int16_t sample2 = 0;
	int32_t result = 0;
	int32_t interpolated1 = 0;
	int32_t interpolated2 = 0;
	int16_t rvb = 0;
//	uint16_t dacdat = 0;
	int8_t volume = 0;
 	Channel_t *channel = &g_channel[0];
	//unint16_t inputdivider = 0;
 	uint8_t instrument = 0;
 	uint32_t frame = 0;
	uint8_t frame_frac = 0;
	uint16_t rate = 0;

	for(channel=&g_channel[0]; channel<=&g_channel[g_trackCount]; channel++){
		instrument = channel->instrument;
	  if(instrument < 255){
	    //Get the current sample : Remember we're using a fixed point setup. The lowest 8 bits are the fractional
			volume = channel->volume;
			frame = channel->position;
			frame_frac = (uint8_t)(frame & 0x000000ff); // fetch the lower 8b
			frame >>= 8; // Drop the lower 8 bits
			rate = channel->rate;
			if(frame < g_song.outpoint[instrument]){
				// Samples left to play
				// Sample value is now a lookup converting from 8bit to 10bit
				sample1 = (int16_t)from8bit_table[g_waveformData[instrument][frame]];
				if(rate == RATE_DEFAULT){
					// Rate is 1:1
					result = sample1;
				}else{
					// Rate is not 1:1 need to interpolate
					sample2 = (int16_t)from8bit_table[g_waveformData[instrument][frame+1]];
					interpolated1 = (sample1 * (0xff - frame_frac));
					interpolated2 = (sample2 * frame_frac);
					result = ((interpolated1 + interpolated2) >> 8);
				}
				// Volume
				result = (int16_t)(result*(16+volume)/16);
				// Add result into output
				output += result;
				// Increment the sample postion by the rate which might not be more or less than a whole frame.
				channel->position += rate;
			}else{
				// End of the instrument
				channel->instrument = 255;
				channel->position = 0;
			}
		}
	}//for

	// Apply reverb if active
	if(g_systemState.reverb){
		// Decay
		rvb = g_reverb_buffer[g_reverb_index] /= 8;
		// Reverb
		rvb += output/4;
		// Clip
		if(rvb > Clip_max){
			rvb = Clip_max;
		}
		if(rvb < Clip_min){
			rvb = Clip_min;
		}
		// Store reverb value back
		g_reverb_buffer[g_reverb_index] = rvb;

		// Increment the reverb buffer
		g_reverb_index++;
		if(g_reverb_index >= REVERB_LENGTH) g_reverb_index = 0;

		// Add previous reverb value into output
		output += g_reverb_buffer[g_reverb_index];
	}

	if(output > Clip_max){
		output = Clip_max;
	}
	if(output < Clip_min){
		output = Clip_min;
	}

	SetDCOC3PWM((uint16_t)(output + PWM_duty_zero));
	mPORTBClearBits(BIT_11);
	INTClearFlag(INT_T1);
}

//_T2Interrupt() is the TMR2 interrupt service routine (ISR). Runs at step freq
void __ISR(_TIMER_2_VECTOR, IPL3SOFT) Timer2Handler(void){
	uint8_t tdx = 0;
	Channel_t *thischannel;
	uint16_t trackData;
	int8_t volume = 0;

	// Load the timer period with the value for the next beat (odd or even)
	if(g_song.step &0b1){
		// Currently playing an even beat
		PR2 = g_song.swing_timer_odd;
	}else{
		// Currently playing an odd beat
		PR2 = g_song.swing_timer_even;
	}

	/*
	 * Output channel stack maintenance.
	 * If a sample has finished playing then remove it 
	 * from the stack and close the gap.
	 * New samples are always added to the top of the stack
	 * if there is a gap.
	 */
	thischannel=&g_channel[0];

	/*
	 * Now increment or reset the step and maybe increment the pattern too
	 */
	if(g_song.step < g_song.patternLength){
		g_song.step++;
	}else{
		g_song.step = 0;
		if(g_systemState.repeat == 0){
			if(g_song.pattern < g_song.length)
				g_song.pattern++;
			else
				g_song.pattern = 0;
		}
	}

	/*
	 * When in play mode we would read the current step of the current
	 * pattern and decide if we need to play any new samples.
	 * If there are samples to play we would add them to the top of the
	 * g_channel stack (if there is space, or not? Undecided as yet.)
	 * We can also play with the volume of each track, 31 step, centered around 0
	 */
	for(tdx=0; tdx<=g_trackCount ;tdx++){
		trackData = g_song.data[g_song.pattern][tdx];
    trackData = trackData >> g_song.step;
		if(trackData & 0b1) {
			//Need to play a sample
			
			if(g_systemState.jamming){
				//Volume management when in Jamming mode
				volume = g_jammode[tdx].volume + g_jammode[tdx].voldir;
				//volume = g_song.volume[tdx] + g_song.voldir[tdx];
				if(volume >= VOLUME_MAX){
					volume = VOLUME_MAX;
					g_jammode[tdx].voldir = -2;
				}
				if(volume < VOLUME_MIN){
					volume = VOLUME_MIN;
					g_jammode[tdx].voldir = 1;
				}
				g_jammode[tdx].volume = volume;
			}else{
				// Not jammode so use the volume in the song
				volume = g_song.volume[tdx];
			}

			//Need to play a sample	: Loop to find the first empty slot
			for(thischannel=&g_channel[0]; thischannel<=&g_channel[CHANNELCOUNT]; thischannel++){
				if(thischannel->instrument == 255){
					thischannel->position = (uint32_t)g_song.inpoint[tdx] << 8;  // Position is 24:8 fractional
					thischannel->instrument = tdx;
					thischannel->volume = volume;
					thischannel->rate = g_song.rate[tdx];
					break;
				}
			}//for
		}
	}//for


	INTClearFlag(INT_T2);
}


//Coretimer interrupt service routine (ISR). Runs at 1000Hz (333hz per row) to gather input from matrix
void __ISR(_CORE_TIMER_VECTOR, IPL3SOFT) CoreTimerHandler(void){
 	/*
		Read inputs, alternate between matrix rows every call
		Inputs are sinks, so setting the row to low activates it
		Sets up the outputs in one int, reads it in the next. 
		Idea is to leave enough time for values to "Settle"
	*/
	// mPORTBSetBits(BIT_10);
	static uint8_t mask = 0b001;
	uint8_t value = (uint8_t)PORTB;

	switch(mask){
		case 0b001:{
			g_input[g_inputstep].stepsHi = value;
			break;
		}
		case 0b010:{
			g_input[g_inputstep].stepsLo = value;
			break;
		}
		case 0b100:{
			g_input[g_inputstep].buttonsHi = value;
			break;
		}
	}

	if(mask == 0b100){
		mask = 0b001;
		//Next index in the ring buffer
		g_inputstep++;
		if(g_inputstep == INPUTRINGLEN) g_inputstep = 0;
	}else{
		mask <<= 1;
	}

	//Setup matrix for next read
	TRISASET = 0b111; //Set all rows to input
	TRISACLR = mask; //Set specific row to output
	
	g_millis++;
	if(Timer1) Timer1--;
	if(Timer2) Timer2--;
	UpdateCoreTimer(TICKS_mS);

	// mPORTBToggleBits(BIT_11);
	INTClearFlag(INT_CT);
}
