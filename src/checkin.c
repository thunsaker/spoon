// 2014 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "common.h"
#include "checkin.h"

void send_checkin_request(char venue_guid[128], char venue_name[512], int private, int twitter, int facebook) {
	if(venue_guid) {
		Tuplet guid_tuple = TupletCString(SPOON_ID, venue_guid);
		Tuplet name_tuple = TupletCString(SPOON_NAME,  venue_name);
		Tuplet private_tuple = TupletInteger(SPOON_PRIVATE, private);
		Tuplet twitter_tuple = TupletInteger(SPOON_TWITTER, twitter);
		Tuplet facebook_tuple = TupletInteger(SPOON_FACEBOOK, facebook);
		
		DictionaryIterator *iter;
		app_message_outbox_begin(&iter);
		
		if(iter == NULL) {
			return;
		}
		
		dict_write_tuplet(iter, &guid_tuple);
		dict_write_tuplet(iter, &name_tuple);
		dict_write_tuplet(iter, &private_tuple);
		dict_write_tuplet(iter, &twitter_tuple);
		dict_write_tuplet(iter, &facebook_tuple);
		dict_write_end(iter);
		
		app_message_outbox_send();
	} else {
		return;
	}
}