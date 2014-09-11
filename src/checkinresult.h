// 2014 Thomas Hunsaker @thunsaker

#pragma once
	
void checkinresult_init(void);
void checkinresult_show(int result, char venue_name[512]);
void checkinresulttip_show(int result, char venue_name[512], char venue_tip[1024]);
void checkinresult_destroy(void);