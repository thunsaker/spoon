// 2016 Thomas Hunsaker @thunsaker

#pragma once
	
#include <pebble.h>
#include <localize.h>

typedef struct {
        char id[25];
        char name[128];
        char address[128];
		char distance[10];
		int distance_unit;
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
	SPOON_BROADCAST = 0x8,
	SPOON_ERROR = 0x9,
	SPOON_CONFIG = 0xA,
	SPOON_RECENT = 0xB,
	SPOON_READY = 0xC,
	SPOON_DISTANCE = 0xD,
	SPOON_UNIT = 0xE
};

enum {
	BROADCAST_DEFAULT = 0,
	BROADCAST_PRIVATE = 1,
	BROADCAST_TWITTER = 2,
	BROADCAST_FACEBOOK = 3,
	BROADCAST_ALL = 4
};

#define KEY_TOKEN 10
#define KEY_THEME 20
#define KEY_UNIT 30
	
#ifdef PBL_SDK_3
	#define STATUS_BAR_OFFSET 0
#else
	#define STATUS_BAR_OFFSET 12
#endif