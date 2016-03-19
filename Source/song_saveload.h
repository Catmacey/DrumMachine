/* 
 * File:   song_saveload.h
 * Author: Matt
 *
 * Created on 16 December 2012, 22:15
 */

#ifndef SONGSAVELOAD_H
#define	SONGSAVELOAD_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

/** P R O T O T Y P E S ******************************************************/
int8_t loadSong( void );
int8_t saveSong( void );
int8_t parseJSON( const char *value );  // Temporarily public for testing

#ifdef	__cplusplus
}
#endif

#endif	/* SONGSAVELOAD_H */

