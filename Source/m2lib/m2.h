/*
 *
 *
 * Menu2 v2.0
 * Martin Hubacek
 * 18.3.2013
 * http://martinhubacek.cz
 *
 *
 */


#ifndef __MENU2__
#define __MENU2__


#include "m2Platform.h"


// Menu flags (16bits)
// 0-3 bits
#define MENU_PARAMETER_MASK 0x0F
#define MENU_PARAMETER_IS_NUMBER 0x1
#define MENU_PARAMETER_IS_STRING 0x2
#define MENU_PARAMETER_IS_FORMATTED 0x4

// 4. bit checkbox bit
#define MENU_ITEM_IS_CHECKBOX	0x10
// 5bit
#define MENU_ITEM_IS_CHECKED	0x20
#define MENU_ITEM_INCREMENT	0x20

// 6.bit - submenu bit
#define MENU_CALLBACK_IS_SUBMENU	0x40

// 7bit - callback bit
#define MENU_CALLBACK_IS_FUNCTION 0x80

// 8bit - inc/dec bit
#define MENU_CALLBACK_IS_INCDEC 0x100

// 9bit - step select (Must press a step button)
#define MENU_CALLBACK_IS_STEPSELECT 0x200

// 10bit - Handoff to aternate process
#define MENU_CALLBACK_IS_HANDOFF 0x400



typedef struct {
	char* text; // The text that is displayed
	uint8_t menuId; // The ID of the menu : Used in the callback : Better than relying on the menu position in menu.items array
	void (*callback)(void *, void *);  // Pointer to function to callback with two pointer arguments
	uint16_t flags; // Flags that define the behaviour of this menu item
	int parameter; // Pointer to an optional value for display alongside text
} MenuItem_t;


typedef struct {
	char* title;  // Title of the menu
	int8_t selectedIndex; // Current active menu item (not just where the cursor but actually selected)
  int8_t cursorIndex; // Current cursor position
  uint8_t lastMenuItem;
  uint8_t menuTopPos;
  uint8_t cursorTopPos;
  uint16_t flags; // Used to toggle the menu routine into different modes : Uses same defines as menuItem.flags
  uint32_t refresh;
	MenuItem_t *items[]; // Array of menu items
} Menu_t;

int8_t menu2(Menu_t *menu, InputRingBuffer_t *input, InputRingBuffer_t *previnput);

#endif
