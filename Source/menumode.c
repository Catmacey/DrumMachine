/*
 * Routines that handle input and display output (LCD and LED) whilst in menu mode
 * Note that menu mode runs in the main() while loop and is executed at approx 100Hz
 * Rendering to the LCD/LED is done at the end of each loop
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
#include "menumode.h"
#include "screen.h"
#include "sequencer.h"
#include "song_saveload.h"
#include "leds.h"
#include "libs/xprintf.h"
#include "m2lib/m2.h"

void callback(void *menu, void *input);

//Used to track current menu mode
typedef enum {
	MENU_DEFAULT,
	MENU_SONGLENGTH,
	MENU_SAVESONG,
	MENU_LOADSONG,
  MENU_JAMMODE,
	MENU_REVERB,
	MENU_TUNING,
	MENU_BACKLIGHT,
	MENU_OTHERTHING,
	MENU_SWING,
} MenuMode_t;

/*
 * Handle processing whilst in menu mode
 * ONLY MENU MODE
 */
	// Simple menu
	MenuItem_t menuItem1 = {
			"Save Song"
		, MENU_SAVESONG
		, callback
		, MENU_CALLBACK_IS_FUNCTION
	};
	MenuItem_t menuItem2 = {
			"Load Song"
		, MENU_LOADSONG
		, callback
		, MENU_CALLBACK_IS_FUNCTION
	};
	MenuItem_t menuItem3 = {
			"Song len"
		, MENU_SONGLENGTH
		, callback
		, MENU_CALLBACK_IS_INCDEC | MENU_PARAMETER_IS_NUMBER
		, (int)&g_song.length
	};
	MenuItem_t menuItem4 = {
			"Jam mode"
		, MENU_JAMMODE
		, callback
		, MENU_ITEM_IS_CHECKBOX | MENU_ITEM_IS_CHECKED | MENU_CALLBACK_IS_FUNCTION
	};
	MenuItem_t menuItem5 = {
			"Reverb"
		, MENU_REVERB
		, callback
		, MENU_ITEM_IS_CHECKBOX | MENU_ITEM_IS_CHECKED | MENU_CALLBACK_IS_FUNCTION
	};
	MenuItem_t menuItem6 = {
			"Tuning"
		, MENU_TUNING
		, callback
		, MENU_CALLBACK_IS_HANDOFF
	};
	MenuItem_t menuItem7 = {
			"Backlight"
		, MENU_BACKLIGHT
		, callback
		, MENU_CALLBACK_IS_INCDEC | MENU_PARAMETER_IS_NUMBER
		, (int)&g_backlight
	};
	MenuItem_t menuItem8 = {
			"Swing"
		, MENU_SWING
		, callback
		, MENU_CALLBACK_IS_INCDEC | MENU_PARAMETER_IS_NUMBER
		, (int)&g_song.swing
	};

	Menu_t simpleMenu = {"Menu mode", .items = {&menuItem2, &menuItem6, &menuItem8, &menuItem3, &menuItem4, &menuItem5, &menuItem7, &menuItem1, 0}};


/*
 * This gets called from the main loop when in menu mode and ONLY on input change
 */
void menu_main(InputRingBuffer_t input, InputRingBuffer_t previnput){
	//MenuMode_t menuMode = g_menuMode;
	// MenuMode_t menuMode;
	int8_t rtn = 0;

  // Simple menu
  // return value is index of the selected item
  if(input.complete){
		// Only do something if a button was pressed
		rtn = menu2(&simpleMenu, &input, &previnput);
		// printf("\nmenuRtn:%d [%d]", rtn, g_millis);
	}

	return;
}


void toggleMenu(void){
	g_systemState.menu = ~g_systemState.menu;
//	g_menuMode = 0;
	if(g_systemState.menu){
		//Light the menu LED
		setLEDstate(MENU, ON);
		simpleMenu.cursorIndex = -1;
		simpleMenu.selectedIndex = -1;
		simpleMenu.cursorTopPos = 0;
		simpleMenu.menuTopPos = 0;
		simpleMenu.flags = 0;
		// Reset the SongExistanceCheck flag
		g_systemState.existanceChecked = 0;
	}else{
		//kill the menu LED
		setLEDstate(MENU, OFF);
		lcd_clearBuffer();
		led_displayPattern();
		displayRunScreen();
	}
}

/*
 * Handles call backs from the menu
 */
void callback(void *menu, void *input){
    //Menu_t* menu = ((Menu_t*)m);
    //MenuItem_t *selectedItem = menu->items[menu->selectedIndex];
		MenuItem_t* selectedItem = ((MenuItem_t*)menu);
		InputRingBuffer_t* inputState = ((InputRingBuffer_t*)input);
		//uint8_t index = selectedItem->menuId;
		char buff[16];
//		int8_t result;
		uint8_t btnIdx;

		static uint8_t state;  // this is the step size in tuning in/out cropping
		static uint8_t mode;  // This is the sub mode of tuning

		
		// printf("\n\nCallback:%d,%s ", selectedItem->menuId, selectedItem->text);

		switch(selectedItem->menuId){
			case MENU_JAMMODE :{
				if(g_systemState.jamming){
					g_systemState.jamming = 0;
				}else{
					g_systemState.jamming = 1;
				}
				xsprintf(buff, "%s:%s", selectedItem->text, g_systemState.jamming?"On":"Off");
				boxOut(buff);
				lcd_render();
				delay_ms(500);
				break;
			}
			case MENU_REVERB :{
				if(g_systemState.reverb){
					g_systemState.reverb = 0;
				}else{
					g_systemState.reverb = 1;
				}
				xsprintf(buff, "%s:%s", selectedItem->text, g_systemState.reverb?"On":"Off");
				boxOut(buff);
				lcd_render();
				delay_ms(500);
				break;
			}
			case MENU_SONGLENGTH :{
				// Is of type MENU_CALLBACK_IS_INCDEC look at MENU_ITEM_INCREMENT for direction
				// printf("Do [%x] %s", selectedItem->flags, selectedItem->flags&MENU_ITEM_INCREMENT?"Inc":"Dec");
				if(selectedItem->flags&MENU_ITEM_INCREMENT)
					songLenInc();
				else
					songLenDec();
				break;
			}
			case MENU_SAVESONG :{
				// Pass the input back to main
				g_systemState.songsave = 1;
				g_systemState.menu = 0;
				break;
			}
			case MENU_LOADSONG :{
				// Pass the input back to main
				g_systemState.songload = 1;
				g_systemState.menu = 0;
				break;
			}
			case MENU_TUNING :{
				// This callback needs to handle the whole tuning interface

				// State is used to switch between hi/low rate of change of the in/out
				// mode 0 = rate
				// mode 1 = volume
				// mode 2 = crop in
				// mode 3 = crop out

				// Get the index of the button that was pressed
				btnIdx = firstBitPos((uint32_t)inputState->steps);

				if(btnIdx){
					// Track selected : Use g_song to store this (note -1 cos instuments are zero based)
					g_song.track = btnIdx-1;
				}

				if(inputState->sel){
					// TODO: Make this three steps
					// TODO: Show the three steps
					// Toggle rate of change
					if(state < 2){
						state++;
					}else{
						state = 0;
					}
				}

				if(inputState->up){
					// Mode up
					if(mode < 3){
						mode++;
					}else{
						mode = 0;
					}
				}
				if(inputState->down){
					if(mode > 0){
						mode--;
					}else{
						mode = 3;
					}
				}
				
				// printf("Tuning mode:%d", btnIdx);
				
				switch(mode){
					case 0:{
						// rate
						if(inputState->left){
							rateModify(g_song.track, -1);
						}	
						if(inputState->right){
							rateModify(g_song.track, 1);
						}
						break;
					}
					case 1:{
						// volume
						if(inputState->left){
							volumeModify(g_song.track, -1);
						}	
						if(inputState->right){
							volumeModify(g_song.track, 1);
						}
						break;
					}
					case 2:{
						// outpoint
						if(inputState->left){
							outpointModify(g_song.track, -(0b10000 << state));
						}	
						if(inputState->right){
							outpointModify(g_song.track, (0b10000 << state));
						}
						break;
					}
					case 3:{
						// inpoint
						if(inputState->left){
							inpointModify(g_song.track, -(0b10000 << state));
						}	
						if(inputState->right){
							inpointModify(g_song.track, (0b10000 << state));
						}
						break;
					}
				}

				if(mode < 2){
					displayTuningGraph(mode & 0b1);
				}else{
					displaySampleCropping((mode & 0b1), state);
				}

				// lcd_setCursor(0,0);
				// xprintf("Mode:%1d", mode);

				lcd_render();
				//delay_ms(1000);

				if(!g_systemState.running && g_channel[g_song.track].instrument == 255){
					// Show which track we're looking at by flashing approriate button led.
					led_displayStepOn(g_song.track);

					//play note if not running
					// printf("Play sample %d Rate:%d, Vol:%d", btnIdx, g_song.ratemod[g_song.track], g_song.volume[g_song.track]);
					g_channel[g_song.track].position = (uint32_t)g_song.inpoint[g_song.track] << 8;  // Posistion is 24:8 fractional
					g_channel[g_song.track].instrument = g_song.track;
					g_channel[g_song.track].volume = g_song.volume[g_song.track];
					g_channel[g_song.track].rate = g_song.rate[g_song.track];
					g_systemState.trackchange = 1;
				}

				break;
			}
			case MENU_BACKLIGHT :{
				// Is of type MENU_CALLBACK_IS_INCDEC look at MENU_ITEM_INCREMENT for direction
				// printf("Do [%x] %s", selectedItem->flags, selectedItem->flags&MENU_ITEM_INCREMENT?"Inc":"Dec");
				if(selectedItem->flags&MENU_ITEM_INCREMENT)
					g_backlight = backlightInc();
				else
					g_backlight = backlightDec();
				break;
			}
			case MENU_SWING :{
				// Is of type MENU_CALLBACK_IS_INCDEC look at MENU_ITEM_INCREMENT for direction
				// printf("Do [%x] %s", selectedItem->flags, selectedItem->flags&MENU_ITEM_INCREMENT?"Inc":"Dec");
				if(selectedItem->flags&MENU_ITEM_INCREMENT){
					swingModify(1);
				}else{
					swingModify(-1);
				}
				break;
			}
			default :{
				break;
			}
		}
}

