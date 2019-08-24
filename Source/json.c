
/* JSON Parsing functions for loading data */

/****** I N C L U D E S **********************************************************/
#include <xc.h>
#include <plib.h>
#include "HardwareConfig.h"
#include <stdint.h>
#include <stdlib.h>
#include "globals.h"
#include "json.h"
#include "sequencer.h"

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
static int8_t handleData( JSON_t *Obj );
static uint8_t parse_trackStrings(JSON_t *Obj);
static const char *parse_value(JSON_t *Obj, const char *value);
static const char *parse_array(JSON_t *Obj, const char *value);
static const char *parse_object(JSON_t *Obj, const char *value);
static const char *parse_number(JSON_t *Obj, const char *num);
static const char *parse_name(JSON_t *Obj, const char *str);
static const char *parse_string(JSON_t *Obj, const char *str);
static const char *parse_generic_string(char *item, const char *str);
// static int8_t peek_value(const char *value);
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
//			g_song.bpm = Obj->valueint;
			bpmSet(Obj->valueint);
			//printf("Ok");
	}else if(strcmp(Obj->name, "length") == 0){
			if(Obj->type != JSON_Number || Obj->valueint < 1 || Obj->valueint > SONGLENMAX+1) return 0; //Wrong type or out of range
			g_song.length = Obj->valueint-1; // We save using human friendly numbers
			//printf("Ok");
	}else if(strcmp(Obj->name, "patternlength") == 0){
			if(Obj->type != JSON_Number || Obj->valueint < 1 || Obj->valueint > PATTERNLENGTHMAX+1) return 0; //Wrong type or out of range
			g_song.patternLength = Obj->valueint-1;
			//printf("Ok");
	}else if(strcmp(Obj->name, "swing") == 0){
			if(Obj->type != JSON_Number || Obj->valueint < SWING_MIN || Obj->valueint > SWING_MAX) return 0; //Wrong type or out of range
//			g_song.swing = Obj->valueint;
			swingSet(Obj->valueint);
			//printf("Ok");
	}else if(strcmp(Obj->name, "trackvolumes") == 0){
			// Track volumes array
			if(Obj->type != JSON_Array) return 0; //Wrong type
			// Clear the volumes
			//memset(g_song.volume, 0, TRACKCOUNT);
			//printf("Ok");
	}else if(strncmp(Obj->name, "trackvolumes.", 13) == 0){
			// The member values of the track volumes array
			//strtok is destructive so work on a copy
			strcpy(name, Obj->name);
			// Discard the first token : Should be "trackvolumes"
			ptr = strtok(name, ".");
			ptr = strtok(NULL, ".");
			if(ptr){
				//First index
				value = (uint8_t)atoi(ptr);
				//printf("\n\tTrackVolume \"%s\" : %d = %d", ptr, value, Obj->valueint);
				if(value > TRACKCOUNT){
					//printf("Ignore surplus track volumes");
					return 0; // Too many patterns
				}
				if(Obj->type != JSON_Number || Obj->valueint < VOLUME_MIN || Obj->valueint > VOLUME_MAX) return 0; //Wrong type or out of range
				g_song.volume[value] = (int8_t)Obj->valueint;
			}
	}else if(strcmp(Obj->name, "trackrates") == 0){
			// Track volumes array
			if(Obj->type != JSON_Array) return 0; //Wrong type
			// Clear the volumes
			//memset(g_song.volume, 0, TRACKCOUNT);
			//printf("Ok");
	}else if(strncmp(Obj->name, "trackrates.", 11) == 0){
			// The member values of the track ratemod array
			//strtok is destructive so work on a copy
			strcpy(name, Obj->name);
			// Discard the first token : Should be "trackrates"
			ptr = strtok(name, ".");
			ptr = strtok(NULL, ".");
			if(ptr){
				//First index
				value = (uint8_t)atoi(ptr);
				//printf("\n\tTrackRate \"%s\" : %d = %d", ptr, value, Obj->valueint);
				if(value > TRACKCOUNT){
					//printf("Ignore surplus track volumes");
					return 0; // Too many patterns
				}
				if(Obj->type != JSON_Number || Obj->valueint < RATEMOD_MIN || Obj->valueint > RATEMOD_MAX) return 0; //Wrong type or out of range
				g_song.ratemod[value] = (int8_t)Obj->valueint;
			}
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

	//printf("\nparse_trackStrings[%d]L[%d]:\"%s\"\n\t=",g_song.track ,g_song.patternLength , Obj->valuestring);

//Add some checks to skip writing once declared pattern length reached.

	step = Obj->valuestring;
	while(*step && ctr++ <= g_song.patternLength){
		trackOutput >>= 1;
		//printf("%d:", ctr);
		if(strncmp(step++, "^", 1) == 0){
			trackOutput |= 0x8000;
			//printf("1 ");
		} else {
			//printf("0 ");
		}
	}
	//printf("\n\tResult = 0x%04x", trackOutput);
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
/*
static int8_t peek_value(const char *value){
	const char *ptr=skip(value);
	//printf("\npeek");
	if (!ptr)						return -1;	// Fail on null.
	//if (!strncmp(value,"null",4))	{ theValue = "";  return value+4; }
	//if (!strncmp(value,"false",5))	{ theValue = ""; return value+5; }
	//if (!strncmp(value,"true",4))	{ theValue = "1";	return value+4; }
	if (*ptr=='\"')				{ return 4; }  // String
	if (*ptr=='-' || (*value>='0' && *value<='9'))	{ return 3; } //Number
	if (*ptr=='[')				{ return 5; } // Array
	if (*ptr=='{')				{ return 6; } // Object
	return -1;	// failure.
}
*/

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
 * return = 1 = success, anything else is failure
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
	g_song.modified = 0;
	printf("\n//parseJSON:Success\n");
	return 1;


}
