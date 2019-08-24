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
int8_t loadSong( uint8_t songindex );
int8_t saveSong( uint8_t songindex );
int8_t song_checkExistance( void );
int8_t song_loadOrSave(uint8_t load, InputRingBuffer_t *input);
// Allows you to set, and therefor see waht the filename will be
void setFilename( uint8_t songIdx );

#ifdef	__cplusplus
}
#endif

#endif	/* SONGSAVELOAD_H */

