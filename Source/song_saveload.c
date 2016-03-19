/*
	Save song in JSON format
	Loops over the current song and saves it out to the SD card in JSON format
	Returns a char to indicate success
	Note that lengths are stored internally as 0 based, so we add 1 to make them more human readable
	0 = success
*/

/****** I N C L U D E S **********************************************************/
#include <xc.h>
#include <plib.h>
#include "HardwareConfig.h"
#include <stdlib.h>
#include <stdint.h>
#include "song_saveload.h"
#include "globals.h"
//#include "cJSON.h"
#include "ff.h"
#include "diskio.h"
#include "xprintf.h"
#include "nokia5110LCD.h"

//File system vars
uint32_t AccSize;			/* Work register for fs command */
uint16_t AccFiles, AccDirs;
FILINFO Finfo;
#if _USE_LFN
char Lfname[512];
#endif
FATFS Fatfs;			/* File system object for each logical drive */
volatile BYTE rtcYear = 110, rtcMon = 10, rtcMday = 15, rtcHour, rtcMin, rtcSec;
FATFS *fs;				/* Pointer to file system object */
DIR dir;				/* Directory object */
FIL file_obj;		/* File objects */

/* JSON Types: */
#define JSON_False 0
#define JSON_True 1
#define JSON_NULL 2
#define JSON_Number 3
#define JSON_String 4
#define JSON_Array 5
#define JSON_Object 6

#define MAXDEPTH 3
#define VALUESIZE 20

// Private typedef

/* The JSON structure: */
typedef struct JSON {
	int8_t type; // The type of the item, as above.
	uint8_t index; // Array index (only applicable for arrays obviously)
	char name[VALUESIZE];
	char valuestring[VALUESIZE];			/* The item's string, if type==JSON_String */
	int valueint;				/* The item's number, if type==JSON_Number */
} JSON_t;

// Private functions
static int8_t loadFile( uint8_t );
static int8_t saveFile( uint8_t );
static int8_t selectSong( void );
//static int8_t parseJSON( const char *value );
static int8_t handleData( JSON_t *Obj );
static uint8_t parse_trackStrings(JSON_t *Obj);
static const char *parse_value(JSON_t *Obj, const char *value);
static const char *parse_array(JSON_t *Obj, const char *value);
static const char *parse_object(JSON_t *Obj, const char *value);
static const char *parse_number(JSON_t *Obj, const char *num);
static const char *parse_name(JSON_t *Obj, const char *str);
static const char *parse_string(JSON_t *Obj, const char *str);
static const char *parse_generic_string(char *item, const char *str);
static int8_t peek_value(const char *value);
static const char *skip(const char *in);

const char *JSON_types[7] = {
	"False"
	,"True"
	,"NULL"
	,"Number"
	,"String"
	,"Array"
	,"Object"
};

/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

DWORD get_fattime (void){
	uint32_t tmr;
	//_DI();
	/* Pack date and time into a uint32_t variable */
	tmr =	  (((uint32_t)rtcYear - 80) << 25)
			| ((uint32_t)rtcMon << 21)
			| ((uint32_t)rtcMday << 16)
			| (uint16_t)(rtcHour << 11)
			| (uint16_t)(rtcMin << 5)
			| (uint16_t)(rtcSec >> 1);
	//_EI();
	return tmr;
}


static void put_rc (FRESULT rc){
	const char *str =
		"OK\0" "DISK\0" "INT\0" "NOT\0" "FILE\0" "PATH\0"
		"INV_NAME\0" "DENIED\0" "EXIST\0" "INV_OBJ\0" "WR_PROT\0"
		"INV_DRV\0" "NOT_EN\0" "NO_FSYS\0" "MKFS_AB\0" "TIMEOUT\0"
		"LOCKED\0" "NO_CORE\0" "TOO_MANY_OPEN\0";
	FRESULT i;

	for (i = 0; i != rc && *str; i++) {
		while (*str++) ;
	}
	xprintf("rc=%u %s\n", (UINT)rc, str);
}



/*
 * Loads a song from SD in JSON format
 * Clears any existing song data.
 * Returns 1 on success.
 */
int8_t loadSong( void ){
	uint8_t result;
	//printf("\nloadSong()");
	lcd_cursorxy(0,0);
	lcd_sendStr("Load Song");
	
	//Choose file
	//songIdx = selectSong();

	//Load file
	result = loadFile(2);
	lcd_cursorxy(0,3);
	if(result)
		lcd_sendStr("Loaded");
	else
		lcd_sendStr("Failed");
	delay_ms(2000);
	return result; //Success
}

/*
 * Saves a song to SD in JSON format
 * Returns  on success.
 */
int8_t saveSong( void ){
	uint8_t result;
	//printf("\nsaveSong()");
	lcd_cursorxy(0,0);
	lcd_sendStr("Save Song:");
	lcd_cursorxy(0,1);
	
	//Choose file

	//Save file
	result = saveFile(2);
	lcd_cursorxy(0,2);
	if(result)
		lcd_sendStr("Saved");
	else
		lcd_sendStr("Failed");
	return result; //Success
}

/*
 * Loads a file into memory
 * Better hope you have a enough free RAM...
 * Should probably allocate a buffer somewhere and enforce max size...
 * Looks for files names songX.jsn where X is 0-15 and provided by arg.songIdx
 * returns 1 to indicate success
*/
static int8_t loadFile( uint8_t songIdx ){
	uint8_t dstatus;

	xsprintf(g_filename, "/song%d.jsn", songIdx);
	lcd_cursorxy(0,1);
	//printf("File:%s", g_filename);
	xprintf("%s", g_filename);

	if(disk_initialize(0)){
	  lcd_sendStr("\nNo Disk!");
		return 0;
	}
	if(f_mount(0, &Fatfs)){
	  lcd_sendStr("\nNo Mount!");
		return 0;
	}

	dstatus = f_open(&file_obj, g_filename, FA_READ);
	if(dstatus){
		put_rc(dstatus);
		return 0;
	}

	lcd_cursorxy(0,2);
	if(file_obj.fsize >= MAXFILESIZE){
		xprintf("\nToo big:%6db", file_obj.fsize);
		printf("\nToo large:%6d bytes ", file_obj.fsize);
		return 0;
	}
	xprintf("\nSize %4dbytes", file_obj.fsize);
	printf("\nLoaded:%6d bytes ", file_obj.fsize);

	f_gets(g_buffer.complete, file_obj.fsize, &file_obj);
	printf("\nFile read into buffer");
	f_close(&file_obj);
	printf("\nFile closed");

	dstatus = parseJSON(g_buffer.complete);

	return dstatus;
}



/*
 * Saves the current song as a file in the SD card
 * Writes files named songX.jsn where X is 0-15 and provided by arg.songIdx
 * returns 1 to indicate success
*/
static int8_t saveFile( uint8_t songIdx){
	uint8_t pattern, track, step, dstatus;

	lcd_clearRam();
	lcd_cursorxy(0,0);
	xprintf("Save:");

	xsprintf(g_filename, "/song%d.jsn", songIdx);

	//printf("File:%s", g_filename);
	xprintf("File:%s", g_filename);

	if(disk_initialize(0)){
	  lcd_sendStr("\nNo Disk!");
		return 0;
	}
	if(f_mount(0, &Fatfs)){
	  lcd_sendStr("\nNo Mount!");
		return 0;
	}

	dstatus = f_open(&file_obj, g_filename, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
	if(dstatus){
		put_rc(dstatus);
		return 0;
	}
	//f_puts("//JSON Formatted song\r\r{\r\t\"saveformat\": 1,\r", &file_obj);
	f_puts("{\r\t\"saveformat\": 1,\r", &file_obj);
	f_printf(&file_obj, "\t\"bpm\": %d,\r", g_song.bpm);
	f_printf(&file_obj, "\t\"length\": %d,\r", g_song.length+1);
	f_printf(&file_obj, "\t\"patternlength\": %d,\r", g_song.patternLength+1);
	f_puts("\t\"patterns\": [\r", &file_obj);
	
	for(pattern=0; pattern<=g_song.length; pattern++){
		f_puts("\t\t[\r", &file_obj);
		//Loop each pattern
		for(track=0; track<=TRACKCOUNT; track++){
			//Each track
			f_puts("\t\t\t\"", &file_obj);
			for(step=0; step<=g_song.patternLength; step++){
				//Each step in the pattern
				if((g_song.data[pattern][track] >> step) & 0b1){
					//beat
					f_puts("^", &file_obj);
				}else{
					//no beat
					f_puts("-", &file_obj);
				}
			}//step
			f_puts("\"", &file_obj);
			if(track<TRACKCOUNT) f_puts(",", &file_obj);
			f_puts("\r", &file_obj);
		}//track
		f_puts("\t\t]", &file_obj);
		if(pattern<g_song.length) f_puts(",", &file_obj);
		f_puts("\r", &file_obj);
	}//pattern

	f_puts("\t]\r", &file_obj);
	f_puts("}\r", &file_obj);
	f_close(&file_obj);	
	return 1;
}


/*
 * Allows you to select 1 of 16 songs to save/load
 * Song selection is via the step buttons
 */
static int8_t selectSong( void ){
	InputRingBuffer_t input, previnput;
	int8_t loop = 1;
	previnput.complete = 0x00000000;
	int8_t songIdx = 0;

	//lcd_cursorxy(0,1);
	//lcd_sendStr("Select");
	/*
	//Wait for user to select a song number (step button) or press menu to exit
	while(loop){
		//Process the input ring buffer
		input = gatherInput();

		if(input.complete != previnput.complete){
			lcd_cursorxy(0,3);
			lcd_sendBitGfx(input.steps, 1);
			lcd_cursorxy(0,4);
			lcd_sendBitGfx(input.buttons, 1);

			//Important stuff first
			if(input.menu){
				loop = 0;
				toggleMenu();
			}

			//Selection of song
			if(input.steps > 0 && input.steps != previnput.steps){
				//Step buttons set the beats
			}
			//Copy state over to prevstate for comparison next iter
			previnput = input;
		}//if

	}//while
	*/
	return songIdx;
}


/* JSON Parsing functions for loading data */


/*
	Handles the data as it is found. Allows us to store or discard it.
	Looks at the Obj
	Need to do something with errors...
*/
static int8_t handleData( JSON_t *Obj ){
	char name[VALUESIZE] = "";
	char *ptr;
	uint8_t value;
	//printf("\nDepth[%d].key[%d] {%6s}:\"%s\"\t", 0, Obj->index, JSON_types[Obj->type], Obj->name);

	switch(Obj->type){
		case JSON_Number: {
			//printf(" Value:%d ", Obj->valueint);
			break;
		}
		case JSON_String: {
			//printf(" Value:\"%s\" ", Obj->valuestring);
			break;
		}
		case JSON_Object:
		case JSON_Array: {
			//printf(" See child ");
			break;
		}
		default: {
			//printf(" [Unknown type] ");
		}
	}

	if(strcmp(Obj->name, "saveformat") == 0){
			if(Obj->type != JSON_Number || Obj->valueint != 1) return 0; //Not the save format I'm expecting
			//printf("Ok");
	}else if(strcmp(Obj->name, "bpm") == 0){
			if(Obj->type != JSON_Number || Obj->valueint < BPMMIN || Obj->valueint > BPMMAX) return 0; //Wrong type or out of range
			g_song.bpm = Obj->valueint;
			//printf("Ok");
	}else if(strcmp(Obj->name, "length") == 0){
			if(Obj->type != JSON_Number || Obj->valueint < 1 || Obj->valueint > SONGLENMAX+1) return 0; //Wrong type or out of range
			g_song.length = Obj->valueint-1; // We save using human friendly numbers
			//printf("Ok");
	}else if(strcmp(Obj->name, "patternlength") == 0){
			if(Obj->type != JSON_Number || Obj->valueint < 1 || Obj->valueint > PATTERNLENGTHMAX+1) return 0; //Wrong type or out of range
			g_song.patternLength = Obj->valueint-1;
			//printf("Ok");
	}else if(strcmp(Obj->name, "patterns") == 0){
			//Start of patterns
			if(Obj->type != JSON_Array) return 0; //Wrong type
			g_song.pattern = 0;
			g_song.track = 0;
			//printf("Ok");
	}else if(strncmp(Obj->name, "patterns.", 9) == 0){
			/*
				An array inside the patterns obj
				Try to determine the index and depth
				Should be stored in the name eg "patterns.001.002"
			*/
			//strtok is destructive so work on a copy
			strcpy(name, Obj->name);
			// Discard the first token : Should be "patterns"
			ptr = strtok(name, ".");
			ptr = strtok(NULL, ".");

			if(ptr){
				//First index : Pattern
				value = (uint8_t)atoi(ptr);
				if(value > SONGLENMAX){
					//printf("Ignore surplus patterns");
					return 0; // Too many patterns
				}
				if(value > g_song.length){
					//printf("More patterns than stated");
					return 0; // Too many patterns
				}
				g_song.pattern = value;
				//printf("\n\tPattern \"%s\" = %d", ptr, value);
				ptr = strtok(NULL, ".");
				if(!ptr){
					// Probably the parent pattern array
					if(Obj->type != JSON_Array) return 0; //Wrong type
				}else{
					//Second index : Track
					if(Obj->type != JSON_String) return 0; //Wrong type
					value = (uint8_t)atoi(ptr);
					//If there are more tracks than we support ignore them
					if(value > TRACKCOUNT){
						//printf("Ignore surplus tracks");
						return 0; // Too many tracks
					}
					//printf("\n\tPattern %d, Track %d", g_song.pattern, value);
					g_song.track = value;
					//Convert the strings into the track data
					parse_trackStrings(Obj);
				}
			}
	}else{
		//printf("Ignore");
	}


	return 1;
};

/*
	Converts the track strings from their string format "^" and "-" to a uint16
	Expects g_song.track and g_song.pattern to already have been set
	Returns 1 for success, 0 for failure
*/
static uint8_t parse_trackStrings(JSON_t *Obj ){
	char *step;
	uint8_t ctr = 0;
	uint16_t trackOutput = 0;

	//printf("\nparse_trackStrings:\"%s\" = ", Obj->valuestring);

//Add some checks to skip writing once declared pattern length reached.

	step = Obj->valuestring;
	while(*step && ctr++ < g_song.patternLength){
		trackOutput >>= 1;
		if(strncmp(step++, "^", 1) == 0){
			trackOutput |= 0x8000;
			//printf("1");
		} else {
			//printf("0");
		}
	}
	//Set the song data
	g_song.data[g_song.pattern][g_song.track] = trackOutput;
	return 1;
}


/* Parse the input text into an unescaped cstring, and populate item. */
static const char *parse_name(JSON_t *Obj, const char *str){
	const char *ptr=str;
	ptr = parse_generic_string(Obj->name, str);
	return ptr;
}
/* Parse the input text into an unescaped cstring, and populate item. */
static const char *parse_string(JSON_t *Obj, const char *str){
	const char *ptr=str;
	ptr = parse_generic_string(Obj->valuestring, str);
	Obj->type = JSON_String;
	handleData(Obj);
	return ptr;
}

/* Parse the input text into an unescaped cstring, and populate item. */
static const char *parse_generic_string(char *item, const char *str){
	//printf("\nparse_string");
	const char *ptr=str+1;char *ptr2;int len=0;
	//printf("\nparse_string(%c) ", *str);
	if (*str!='\"') {return 0;}	/* not a string! */

	while (*ptr!='\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++;	/* Skip escaped quotes. */

	if(len >= VALUESIZE) return 0; /*String too long for buffer*/

	ptr=str+1;ptr2=item;
	while (*ptr!='\"' && *ptr)
	{
		//printf(" %c", *ptr);
		if (*ptr!='\\') *ptr2++=*ptr++;
		else
		{
			ptr++;
			switch (*ptr)
			{
				case 'b': *ptr2++='\b';	break;
				case 'f': *ptr2++='\f';	break;
				case 'n': *ptr2++='\n';	break;
				case 'r': *ptr2++='\r';	break;
				case 't': *ptr2++='\t';	break;
				//removed unicode handling
				default:  *ptr2++=*ptr; break;
			}
			ptr++;
		}
	}
	*ptr2=0;
	if (*ptr=='\"') ptr++;
	return ptr;
}

/* Parser core - when encountering text, process appropriately. */
static const char *parse_value(JSON_t *Obj, const char *value){
	//printf("\nparse_value");
	if (!value)						return 0;	/* Fail on null. */
	//if (!strncmp(value,"null",4))	{ theValue = "";  return value+4; }
	//if (!strncmp(value,"false",5))	{ theValue = ""; return value+5; }
	//if (!strncmp(value,"true",4))	{ theValue = "1";	return value+4; }
	if (*value=='\"')				{ return parse_string(Obj, value); }
	if (*value=='-' || (*value>='0' && *value<='9'))	{ return parse_number(Obj, value); }
	if (*value=='[')				{ return parse_array(Obj, value); }
	if (*value=='{')				{ return parse_object(Obj, value); }

	return 0;	/* failure. */
}

/*
	Peeks at the value to return a JSON_t type
	Doesn't alter the pointer position
	Returns a valid JSON_t type index or -1
*/
static int8_t peek_value(const char *value){
	const char *ptr=skip(value);
	//printf("\npeek");
	if (!ptr)						return -1;	/* Fail on null. */
	//if (!strncmp(value,"null",4))	{ theValue = "";  return value+4; }
	//if (!strncmp(value,"false",5))	{ theValue = ""; return value+5; }
	//if (!strncmp(value,"true",4))	{ theValue = "1";	return value+4; }
	if (*ptr=='\"')				{ return 4; }  // String
	if (*ptr=='-' || (*value>='0' && *value<='9'))	{ return 3; } //Number
	if (*ptr=='[')				{ return 5; } // Array
	if (*ptr=='{')				{ return 6; } // Object
	return -1;	/* failure. */
}


/* Parse the input text to generate a number, and populate the result into item. */
static const char *parse_number(JSON_t *Obj, const char *num)
{
	signed int n=0;
	signed char sign=1;
	if (*num=='-') sign=-1,num++;	/* Has sign? */
	if (*num=='0') num++;			/* is zero */
	if (*num>='1' && *num<='9')	do	n=(n*10)+(*num++ -'0');	while (*num>='0' && *num<='9');	/* Number? */
	n=(int)sign*n;
	Obj->valueint = n;
	Obj->type = JSON_Number;
	handleData(Obj);
	return num;
}



/* Build an object from the text. */
static const char *parse_object(JSON_t *Obj, const char *value){
	int8_t idx = 0;
	//printf("\nparse_object");
	if (*value!='{')	{return 0;}	/* not an object! */
	Obj->type = JSON_Object;
	Obj->index = idx;
	value = skip(value+1);
	if (*value=='}') return value+1;	/* empty array. */

	//Now we have a name and a value we should look to store it somewhere
	handleData(Obj);

	//Child values are one object deeper
	Obj++;
	Obj->index = idx;

	//Get the name of the first key in the object
	value = skip(parse_name(Obj, skip(value)));
	if (!value) return 0;

	//Get the value of the first key
	if (*value!=':') {return 0;}	/* fail! */

	value=skip(parse_value(Obj, skip(value+1)));	/* skip any spacing, get the value. */
	if (!value) return 0;

	idx++;

	//Next key (if any)
	while (*value==','){
		//Get the name of the next key in the object
		value = skip(parse_name(Obj, skip(value+1)));
		if (!value) return 0;

		if (*value!=':') {return 0;}	/* fail! */
		value = skip(parse_value(Obj, skip(value+1)));	/* skip any spacing, get the value. */
		if (!value) return 0;
		Obj->index = idx;

		idx++;
	}

	if (*value=='}') return value+1;	/* end of array */
	return 0;	/* malformed. */
}

/* Build an array from input text. */
static const char *parse_array(JSON_t *Obj, const char *value){
	const char *ptr = Obj->name;
	char name[VALUESIZE] = "";
	uint8_t len = 0;
	uint8_t idx = 0;

	if (*value!='[')	{return 0;}	/* not an array! */

	Obj->type = JSON_Array;
	Obj->valueint = 0;
	strncpy(Obj->valuestring, "", VALUESIZE);

	while (*ptr++ && (len<VALUESIZE))	len++;	//Get length of current Obj.name
	//Copy the name here
	strncpy(name, Obj->name, len);

	value = skip(value+1);
	if (*value==']') return value+1;	/* empty array. */

	//Array parent
	handleData(Obj);

	//Child values are one object deeper
	Obj++;
	//Make the name of the next object contain the index
	Obj->index = idx++;
	snprintf(Obj->name, VALUESIZE, "%s.%02d", name, Obj->index);


	value = skip(parse_value(Obj, skip(value)));	/* skip any spacing, get the value. */
	if (!value) return 0;

	while (*value==','){
		//Make the name of the next object contain the index
		Obj->index = idx++;
		snprintf(Obj->name, VALUESIZE, "%s.%02d", name, Obj->index);
		value = skip(parse_value(Obj, skip(value+1)));	/* skip any spacing, get the value. */
		if (!value) return 0;
	}

	if (*value==']'){
		return value+1;	/* end of array */
	}
	return 0;	/* malformed. */
}


/* Utility to jump whitespace and cr/lf */
static const char *skip(const char *in) {while (in && *in && (unsigned char)*in<=32) in++; return in;}


/*
 * Converts the JSON String into song data
 * Is a very simplified JSON parser (Very simple)
 */
int8_t parseJSON(const char *value){
	const char *result = 0;

	//printf("parseJSON() : sizeof(JSON)=%d", sizeof(JSON_t));

	// Create the array of JSON_t Objects here in the stack
	JSON_t Data[MAXDEPTH+1];
	// Create pointer to the first one.
	JSON_t *Obj = &Data[0];
	printf("\nparseJSON");
	result = parse_value(Obj, value);
	if(!result){
		printf("\n//Failed");
		return 0;
	}
	//Set the counters back to zero
	g_song.pattern = 0;
	g_song.step = 0;
	g_song.track = 0;
	printf("\n//parseJSON:Success\n");
	return 1;


}