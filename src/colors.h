// 2015 Thomas Hunsaker @thunsaker

#pragma once

#include <pebble.h>

uint8_t get_primary_color();
void set_primary_color(uint8_t color);
uint8_t get_accent_color();
void set_accent_color(uint8_t color);
uint8_t get_back_color();
void set_back_color(uint8_t color);
void colors_init(uint8_t primary, uint8_t accent, uint8_t back);