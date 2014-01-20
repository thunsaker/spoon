// 2014 Thomas Hunsaker @thunsaker

#pragma once

void send_checkin_request_confirmation(char venue_guid[128], char venue_name[512], int private);
void venueconfirmation_init(void);
void venueconfirmation_show(char venue_guid[128], char venue_name[512]);
void venueconfirmation_destroy(void);
bool venueconfirmation_is_on_top();