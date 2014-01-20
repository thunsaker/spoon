// 2014 Thomas Hunsaker @thunsaker
// Heavily modified version of Neal's Hacker News list implementation - https://github.com/Neal/pebble-hackernews

#pragma once

void venuelist_init(void);
void venuelist_show();
void venuelist_destroy(void);
void venuelist_in_received_handler(DictionaryIterator *iter);
bool venuelist_is_on_top();