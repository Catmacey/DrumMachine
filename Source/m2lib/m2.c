
/*
 *
 * Menu code based on Menu2 v2.0 by Martin Hubacek
 * http://martinhubacek.cz
 *
 */
#include <xc.h>
#include <plib.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "../HardwareConfig.h"
#include "../globals.h"
#include "../lcd/lcd.h"
#include "../menumode.h"
#include "../libs/xprintf.h"
#include "../screen.h"
#include "m2.h"

// NEED TO FIX THE "Shall I draw menu" logic.  Redraws on Down but not on Up!

int8_t menu2(Menu_t *menu, InputRingBuffer_t *input, InputRingBuffer_t *previnput){
	uint8_t idx, len = 0, index, posx, row;
	//int8_t lastMenuItem = menu->cursorIndex;
//	uint8_t cursorTopPos = menu->cursorTopPos;
	uint8_t menuTopPos = menu->menuTopPos;
	uint16_t flags;
	//char buff[16];
	//uint32_t refreshTimer = 0;
	MenuItem_t **iList = menu->items;

	menu->lastMenuItem = menu->cursorIndex;
	if(menu->cursorIndex == -1) menu->cursorIndex = 0;

	printf("\n\nMenu2():Cursor:%d, Selected:%d", menu->cursorIndex, menu->selectedIndex);
	// if(menu->selectedIndex > -1) printf(", Flags:%d", menu->items[menu->selectedIndex]->flags);

	//
	// Get number of items in menu, search for the first NULL
	//
	for (; *iList != 0; ++iList){
	  len++;
	}

	// Functional :)
	// menuItem, menuTopPos, cursorTopPos
	/*
	if(menu->selectedIndex > -1){
		printf("\nWeird sh*t");
	  // If item on the first screen
	  if(menu->selectedIndex < MENU_LINES) {
			printf(" A");
		  menu->cursorIndex = menu->selectedIndex;
		  cursorTopPos = menu->selectedIndex;
		  menuTopPos = 0;
	  } else {
			printf(" B");
		 // Item is on other screen
		  menu->cursorIndex = menu->selectedIndex;
		  cursorTopPos = MENU_LINES - 1;
		  menuTopPos =(uint8_t)menu->selectedIndex - cursorTopPos;
	  }
	}
*/
	
	if(menu->flags & MENU_CALLBACK_IS_INCDEC){
		// printf("\nInc Dec mode");
		// In this mode we don't navigate the menu we call the call back
		// We set the inc/dec flag (shared with checked) and callback
		// Left gets us out
		//
		// Up
		//
		if(menu->selectedIndex < 0){
			// printf("Fail! selectedIndex must be greater than -1");
			return -1;
		}
		if(input->up || input->down){
			if(input->up){
				// printf("\ninc");
				menu->items[menu->selectedIndex]->flags |= MENU_ITEM_INCREMENT;
			}
			if(input->down){
				// printf("\ndec");
				menu->items[menu->selectedIndex]->flags &= ~MENU_ITEM_INCREMENT;
			}
			// Call back
			(*menu->items[menu->selectedIndex]->callback)(menu->items[menu->selectedIndex], input);
			// Force refresh
			menu->lastMenuItem = -1;
		}
	}else if(menu->flags & MENU_CALLBACK_IS_STEPSELECT){
		// Step select mode
		// User must press a step button
		// Left gets us out
		// printf("\nStep select mode");
		if(input->steps && countBits((uint32_t)input->steps) == 1){
			// A single step button was pressed
			// printf("\nStep:%d", input->steps);
			// Call the callback function
			(*menu->items[menu->selectedIndex]->callback)(menu->items[menu->selectedIndex], input);
		}
	}else if(menu->flags & MENU_CALLBACK_IS_HANDOFF){
		// Handoff control to an alternate process.
		// It need to manage it's own display drawing and menu exit
		// printf("\nHandoff mode");
		// Call the callback function
		(*menu->items[menu->selectedIndex]->callback)(menu->items[menu->selectedIndex], input);
		return menu->cursorIndex;
	}else{
		// Normal menu nav mode
		// printf("\nNormal nav mode");

		// Down
		if(input->down){
			// printf("\ndown");
			if(menu->cursorIndex != len-1){
				// move to next item
				menu->cursorIndex++;
				/*
				if(cursorTopPos >= MENU_LINES-1 || (cursorTopPos == ((MENU_LINES)/2) && ((len) - menu->cursorIndex  ) > ((MENU_LINES-1)/2)) ) {
				  menuTopPos++;
				}else{
				  cursorTopPos++;
				}
				*/
			}else{
				// Last item in menu => go to first item
				menu->cursorIndex = 0;
				//cursorTopPos = 0;
				//menuTopPos = 0;
			}
		}

		// Up
		if(input->up){
			// printf("\nup ");
			// printf("menuItem:%d ", menu->cursorIndex);
			if(menu->cursorIndex != 0){
				menu->cursorIndex--;
				/*
				if(cursorTopPos > 0 && !((cursorTopPos == MENU_LINES/2) && (menu->cursorIndex >= MENU_LINES/2))){
					cursorTopPos--;
				}else{
					menuTopPos--;
				}
				*/
			}else{
				// go to the last item in menu
				menu->cursorIndex = len-1;
				/*
				if(len <= MENU_LINES){
					menuTopPos = 0;
				} else {
					menuTopPos = menu->cursorIndex;
				}
				if(menuTopPos > len - MENU_LINES && len >= MENU_LINES){
					menuTopPos = len - MENU_LINES;
				}
				cursorTopPos = menu->cursorIndex - menuTopPos;
				*/
			}
			// printf("menuItem:%d ", menu->cursorIndex);
		}

	  // Enter
		if(input->right){
			// printf("\nRight: ");
			menu->selectedIndex = menu->cursorIndex;
			flags = menu->items[menu->cursorIndex]->flags;

			// checkbox
			if(flags & MENU_ITEM_IS_CHECKBOX){
				// printf("Checkbox");
				menu->items[menu->cursorIndex]->flags ^= MENU_ITEM_IS_CHECKED;
				// Force refresh
				menu->lastMenuItem = -1;
				menu->selectedIndex = -1;
			}

			// Item is submenu - parameter in callback
			if(flags & MENU_CALLBACK_IS_SUBMENU && menu->items[menu->cursorIndex]->callback){
				// printf("Submenu");
				menu2((Menu_t*)menu->items[menu->cursorIndex]->callback, input, previnput);
				// Force refresh
				menu->lastMenuItem = -1;
			}

			// callback
			if(flags & MENU_CALLBACK_IS_FUNCTION && menu->items[menu->cursorIndex]->callback){
				// printf("Callback");
				(*menu->items[menu->cursorIndex]->callback)(menu->items[menu->cursorIndex], input);
				// Force refresh
				menu->lastMenuItem = -1;
			}

			// Increment/Decrement a value via callback using up/down
			if(flags & MENU_CALLBACK_IS_INCDEC && menu->items[menu->cursorIndex]->callback){
				// printf("Inc/Dec");
				// Just set the mode, don't callback yet
				//(*menu->items[menu->cursorIndex]->callback)(menu);
				menu->flags = MENU_CALLBACK_IS_INCDEC;
				// Force refresh
				menu->lastMenuItem = -1;
			}

			// Requires the user to press one of the step buttons
			if(flags & MENU_CALLBACK_IS_STEPSELECT && menu->items[menu->cursorIndex]->callback){
				// printf("Step select");
				// Just set the mode, don't callback yet
				menu->flags = MENU_CALLBACK_IS_STEPSELECT;
				// Display the dialog
				boxOutWithTitle(menu->items[menu->cursorIndex]->text, "Choose a step");
				lcd_render();
				// Force refresh
				//menu->lastMenuItem = -1;
			}

			// Hands off all control to external routine : Only way out is to press MENU
			if(flags & MENU_CALLBACK_IS_HANDOFF && menu->items[menu->cursorIndex]->callback){
				// printf("Handoff");
				// Just set the mode, don't callback yet
				menu->flags = MENU_CALLBACK_IS_HANDOFF;
				// Display the dialog
				boxOutWithTitle(menu->items[menu->cursorIndex]->text, "Use Joy & Steps");
				lcd_render();

			}

			// normal item, so exit
			if((menu->items[menu->cursorIndex]->callback == 0) && ((flags & MENU_ITEM_IS_CHECKBOX) == 0)){
				// printf("Nada");
				return menu->cursorIndex;
			}
		}
	}
	


	//
	// Left - back
	//
	if(input->left){
		// printf("\nleft");
		//keyPress = 0;
		menu->selectedIndex = -1;
		menu->lastMenuItem = -1;
		menu->flags = 0;
		// return -1;
	}


		//
		// If menu item changed -> refresh screen
		//
//		if(menu->lastMenuItem != menu->cursorIndex || (menu->refresh && MENU_MS_TICK > refreshTimer)){
//			if(menu->refresh){
//				refreshTimer = MENU_MS_TICK + menu->refresh;
//			}
		// Always draw the menu
		if(menu->lastMenuItem != menu->cursorIndex){
		  displayClear();
		  // displayString(menu->title,0,0);

		  // Menu debug
		  // printf("\nDISP len[%d], menuItem:%d, selIdx:%d, menuTopPos:%d, cursorTopPos:%d\n", len, menu->cursorIndex, menu->selectedIndex, menuTopPos, cursorTopPos);
		  //displayString(buffer,0,0);

		  idx = 0;
		  while((idx + menuTopPos) < len && idx < MENU_LINES){
				index = menuTopPos + idx;
				row = idx;
				// printf("%d:%d ", idx, index);
				if(menu->cursorIndex == index && MENU_LINES > 1){
					if(menu->cursorIndex == menu->selectedIndex){
						// Bolder arrow
						displayString("\x84", 0, ROW(row));
					}else{
						//Normal arrow
						displayString(ARROW_SYMBOL, 0, ROW(row));
					}

				}

				//uint8_t posx = strlen(menu->items[index]->text) + 3;
				//Always draw result in same place
				posx = 12;

				if(MENU_LINES > 1){
					displayString(menu->items[index]->text, COL(1+ARROW_GAP), ROW(row));
				} else {
					displayString(menu->items[index]->text, COL(0), ROW(row));
				}
				//Not sure of the advantage of this
				//if((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_FORMATTED){
					// This overwrites the menu text again but I don't care for now.
					//lcd_setCursor(2, ROW(row));
					//xprintf(menu->items[index]->text, (uint8_t)*((int*)menu->items[index]->parameter));
					//displayFormatted(menu->items[index], COL(posx), ROW(row));
				//}
				if((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_NUMBER){
					displayNumber((uint8_t)*((int*)menu->items[index]->parameter), COL(posx), ROW(row));
				}
				if((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_STRING){
					displayString((char*)(menu->items[index]->parameter), COL(posx),ROW(row));
				}
				if(menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX) {
					if(menu->items[index]->flags & MENU_ITEM_IS_CHECKED){
						displayString("\x83", COL(1),ROW(row));
					}else{
						displayString("\x82", COL(1),ROW(row));
					}
				}
				idx++;
			}

			#ifdef displayDraw
			  displayDraw();
			#endif

		  menu->lastMenuItem = menu->cursorIndex;
		}


	//}

	return 0;

}
	