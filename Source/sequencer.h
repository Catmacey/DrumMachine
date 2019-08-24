/* 
 * File:   sequencer.h
 * Author: Matt
 *
 * Created on 21 September 2013, 19:35
 */

#ifndef SEQUENCER_H
#define	SEQUENCER_H

#ifdef	__cplusplus
extern "C" {
#endif

void setupSampleTracks(void);
void seq_run(void);
void seq_stop(void);
void seq_toggleRepeat(void);
void seq_toggle(void);
void patternNext(void);
void patternPrev(void);
void trackPrev(void);
void trackNext(void);
void bpmUp(void);
void bpmDown(void);
void bpmModify(int8_t value);
void bpmSet(uint8_t value);
void bpmSetTmr(void);
void songLenInc(void);
void songLenDec(void);
int16_t calcRate(int8_t ratemod);
void inpointModify(uint8_t track, int8_t value);
void outpointModify(uint8_t track, int8_t value);
void rateModify(uint8_t track, int8_t value);
void rateUp(uint8_t track);
void rateDown(uint8_t track);
void volumeModify(uint8_t track, int8_t value);
void volumeDown(uint8_t track);
void volumeUp(uint8_t track);
void swingModify(int8_t value);
void swingSet(uint8_t value);


#ifdef	__cplusplus
}
#endif

#endif	/* SEQUENCER_H */

