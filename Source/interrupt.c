#include <xc.h>
#include <plib.h>
#include "HardwareConfig.h"
#include "globals.h"


//_T1Interrupt() is the T1 interrupt service routine (ISR). Runs at sample freq
void __ISR(_TIMER_1_VECTOR, ipl5) Timer1Handler(void){
	//channel freqs
	signed short output = 0;
	signed short sample = 0;
	signed char volume = 0;
 	Channel_t *channel = &g_channel[0];
	//unsigned short inputdivider = 0;
 	unsigned char instrument = 0;
 	unsigned long frame = 0;

	for(channel=&g_channel[0]; channel<=&g_channel[g_trackCount]; channel++){
		instrument = channel->instrument;
	  if(instrument < 255){
	    //Get the current sample
			frame = channel->position++;
			volume = channel->volume;
			sample = g_sampData[instrument][frame];
			//Volume
			sample = (sample*(16+volume)/16);

			output += (signed short)sample;
			if(frame >= g_sampLen[instrument]){
				channel->instrument = 255;
				channel->position = 0;
			}
		}
	}//for

	if(output > Clip_max){
		output = Clip_max;
		//CLIP = 0;
	}
	if(output < Clip_min){
		output = Clip_min;
		//CLIP = 0;
	}
  //Now it's clipped to -128 <= 128
  //output += PWM_8bit_zero;

	SetDCOC3PWM((unsigned char)(output + PWM_duty_zero));

	INTClearFlag(INT_T1);
}

//_T2Interrupt() is the TMR2 interrupt service routine (ISR). Runs at step freq
void __ISR(_TIMER_2_VECTOR, ipl3) Timer2Handler(void){
	unsigned char tdx = 0;
	Channel_t *thischannel ;//,*nextchannel *tmpchannel;
	unsigned short trackData;
	signed char volume = 0;
	//signed char voldir = 0;
	
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
			//Need to play a sample	: Loop to find the first empty slot

			if(g_systemState.jamming){
				//Volume management when in Jamming mode
				volume = g_song.volume[tdx] + g_song.voldir[tdx];
				if(volume >= VOLUME_MAX){
					volume = VOLUME_MAX;
					g_song.voldir[tdx] = -2;
				}
				if(volume < VOLUME_MIN){
					volume = VOLUME_MIN;
					g_song.voldir[tdx] = 1;
				}
				g_song.volume[tdx] = volume;
				//g_song.volume[tdx] += g_song.voldir[tdx];
			}else{
				volume = 0;
			}

			for(thischannel=&g_channel[0]; thischannel<=&g_channel[CHANNELCOUNT]; thischannel++){
				if(thischannel->instrument == 255){
					thischannel->position = 0;
					thischannel->instrument = tdx;
					//cycle fade mode : Each instrument fades in and out on it's own
					thischannel->volume = volume;
					break;
				}
			}//for
		}
	}//for
		
	INTClearFlag(INT_T2);
}


//_T1Interrupt() is the T4 interrupt service routine (ISR). Runs at around 150hz to gather input from matrix
void __ISR(_TIMER_4_VECTOR, ipl5) Timer4Handler(void){
 	/*
		Read inputs, alternate between matrix rows every call
		Inputs are sinks, so setting the row to low activates it
		Sets up the outputs in one int, reads it in the next. 
		Idea is to leave enough time for values to "Settle"
	*/
	unsigned char mask;
	unsigned char value = (unsigned char)PORTB & 0x000000FF;

	//First read the value on lower 8 bits of port b and store
	if(g_matrixrow == 0){
		//Read previous setup
		g_input[g_inputstep].stepsHi = value;
		//Setup for row 1
		mask = 0b010;		
	}else if (g_matrixrow == 1){
		g_input[g_inputstep].stepsLo = value;
		mask = 0b100;
	}else if (g_matrixrow == 2){
		g_input[g_inputstep].buttonsHi = value;
		mask = 0b001;
	}

	//Setup matrix for next read
	TRISASET = 0b111; //Set all rows to input
	//LATASET = 0b111; //Set all high
	//LATACLR = mask; //Set specific row low
	TRISACLR = mask; //Set specific row to output

	g_matrixrow++;
	if(g_matrixrow > 2){
		g_matrixrow = 0;
		//Next index in the ring buffer
		g_inputstep++;
		if(g_inputstep == INPUTRINGLEN) g_inputstep = 0;
	}
	INTClearFlag(INT_T4);
}

//Coretimer interrupt service routine (ISR). Runs at 1000Hz
void __ISR(_CORE_TIMER_VECTOR, ipl6) CoreTimerHandler(void){
	//
	g_millis++;
	if(Timer1) Timer1--;
	if(Timer2) Timer2--;
	UpdateCoreTimer(TICKS_mS);
	INTClearFlag(INT_CT);
}
