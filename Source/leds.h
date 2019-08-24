/* 
 * File:   leds.h
 * Author: Matt
 *
 * Created on 25 July 2013, 00:01
 *
 * Contains all the functions that control the LEDs
 * Uses functions in maxim6957.h
 *
 */

#ifndef LEDS_H
#define	LEDS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define ON 0x01
#define OFF 0x00

/*
 * An ENUM of LED names in the order as thier actual ports
 * We start at port 12 on this device so have to add an offset
 * Used in setLEDintensity() and setLEDstate()
 */
typedef enum {
      STEP1 	// Port 12
    , STEP2 	// Port 13
    , STEP3 	// Port 14
    , LCD_BL 	// Port 15
    , STEP9 	// Port 16
    , STEP10 	// Port 17
    , STEP11 	// Port 18
    , STEP13 	// Port 19
    , STEP12 	// Port 20
    , STEP4 	// Port 21
    , STEP5 	// Port 22
    , STEP14 	// Port 23
    , STEP15 	// Port 24
    , STEP16 	// Port 25
    ,	RUN 		// Port 26
    , STEP6 	// Port 27
    ,	REPEAT 	// Port 28
    , STEP7 	// Port 29
    , MENU 		// Port 30
    , STEP8		// Port 31
    } lednames_t;

//Pattern default LED intensity (0 <= X <= 15)
#define LED_INTENSITY_BRIGHT 15
#define LED_INTENSITY_DEFAULT 1
#define LED_INTENSITY_MAX 15
#define LED_INTENSITY_MIN 0


/** I N C L U D E S **********************************************************/

#include "libs/maxim6957.h"


/** P R O T O T Y P E S ******************************************************/
void clearStepLED();
void max6957setup(void);
void led_displayPattern(void);
void led_displayStepNext(int8_t);
void led_displayStepOn(int8_t);
void led_displayStepOff(int8_t);
void setLEDstate(lednames_t, unsigned char);
void setLEDintensity(lednames_t, unsigned char);
uint8_t getLEDintensity(lednames_t led);
uint8_t backlightInc( void );
uint8_t backlightDec( void );

/** V A R I A B L E S ******************************************************/

extern const uint8_t g_maxPortMap[];


#ifdef	__cplusplus
}
#endif

#endif	/* LEDS_H */

