// 2014 Thomas Hunsaker @thunsaker

#pragma once

typedef struct {
        char id[25];
        char name[128];
        char address[128];
		int index;
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
	SPOON_ERROR = 0xB
};