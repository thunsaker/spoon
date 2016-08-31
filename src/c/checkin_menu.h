// 2016 Thomas Hunsaker @thunsaker

#pragma once
	
#include <pebble.h>

void checkin_menu_show(bool menu_mode, char venue_guid[128], char venue_name[512]);
bool checkin_menu_is_on_top();
void checkin_menu_deinit(void);