// 2016 Thomas Hunsaker @thunsaker

#pragma once
	
#include <pebble.h>

#define CHECKIN_COUNTDOWN_BAR_SIZE GSize(144,1)
#define CHECKIN_COUNTDOWN_DELTA 100
	
void checkin_show(void);
void checkin_send_request(char venue_guid[128], char venue_name[512], int private, int twitter, int facebook, bool show_checkin);
void checkin_in_received_handler(DictionaryIterator *iter);
void checkin_result_receiver(bool result);
void checkin_deinit(void);