/* 
 * File:   screen.h
 * Author: Matt
 *
 * Created on 24 July 2013, 00:10
 *
 * Routines that draw to the screen
 *
 */

#ifndef SCREEN_H
#define	SCREEN_H

#ifdef	__cplusplus
extern "C" {
#endif

#define LCDTYPE_NOKIA_1202
//#define LCDTYPE_NOKIA_5110

/** P R O T O T Y P E S ******************************************************/
void displayRunScreen(void);
void displayStep(void);
void displayBPM(void);
void displayTrackVolumes(uint8_t top, uint8_t height, int8_t selected);
void displaySampleCropping(uint8_t cursor, uint8_t stepsize);
void displayPatternGraph(uint8_t tall);
void lcdBLOn(void);
void lcdBLOff(void);
void displayPatternTrackCounters(void);
void printBits(size_t const, void const * const);
void drawVerticalSlider(uint8_t left, uint8_t top, uint8_t width, uint8_t height, uint8_t max, uint8_t value);
void drawHorizontalSlider(uint8_t left, uint8_t top, uint8_t width, uint8_t height, uint16_t max, uint16_t value);
void drawInOutPoints(uint8_t left, uint8_t top, uint8_t width, uint8_t height, uint16_t max, uint16_t inpoint, uint16_t outpoint);
void boxOut(char *text);
void boxOutWithTitle(char *title, char *text);
void displayTuningGraph(uint8_t state);
void displayExistingSongs(uint8_t load);

#ifdef	__cplusplus
}
#endif

#endif	/* SCREEN_H */

