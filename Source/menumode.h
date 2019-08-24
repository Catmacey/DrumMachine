/* 
 * File:   menu.h
 * Author: Matt
 *
 * Created on 21 September 2013, 20:00
 */

#ifndef MENU_H
#define	MENU_H

#ifdef	__cplusplus
extern "C" {
#endif

void toggleMenu(void);
void menu_main(InputRingBuffer_t input, InputRingBuffer_t previnput);

#ifdef	__cplusplus
}
#endif

#endif	/* MENU_H */

