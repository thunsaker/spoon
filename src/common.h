// 2015 Thomas Hunsaker @thunsaker

#pragma once
	
#include <pebble.h>

typedef struct {
        char id[25];
        char name[128];
        char address[128];
		int index;
		bool isRecent;
} SpoonVenue;

enum {
	SPOON_TOKEN = 0x0,
	SPOON_LOCATION = 0x1,
	SPOON_INDEX = 0x2,
	SPOON_ID = 0x3,
	SPOON_NAME = 0x4,
	SPOON_ADDRESS = 0x5,
	SPOON_REFRESH = 0x6,
	SPOON_RESULT = 0x7,
	SPOON_PRIVATE = 0x8,
	SPOON_TWITTER = 0x9,
	SPOON_FACEBOOK = 0xA,
	SPOON_ERROR = 0xB,
	SPOON_CONFIG = 0xC,
	SPOON_RECENT = 0xD,
	SPOON_READY = 0xE
};

#define KEY_TOKEN 10
#define KEY_THEME 20
	
#define SCREEN_WIDTH 144
#ifdef PBL_SDK_3
	#define SCREEN_HEIGHT 168
	#define STATUS_BAR_OFFSET 0
#else
	#define SCREEN_HEIGHT 156
	#define STATUS_BAR_OFFSET 12
#endif
	
#define DIALOG_MESSAGE_NOT_CONNECTED  "Connect to Foursquare on Phone"
#define DIALOG_MESSAGE_NO_PHONE "Error:\n No connection to phone"
#define DIALOG_MESSAGE_NO_LOCATION  "No Location"