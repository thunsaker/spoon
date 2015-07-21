// 2015 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "colors.h"

static uint8_t primary_color;
static uint8_t accent_color;
static uint8_t back_color;

uint8_t get_primary_color() {
	return primary_color;
}

void set_primary_color(uint8_t color) {
	primary_color = color;
}

uint8_t get_accent_color() {
	return accent_color;
}

void set_accent_color(uint8_t color) {
	accent_color = color;
}

uint8_t get_back_color() {
	return back_color;
}

void set_back_color(uint8_t color) {
	back_color = color;
}

void colors_init(uint8_t primary, uint8_t accent, uint8_t back) {
	set_primary_color(primary);
	set_accent_color(accent);
	set_back_color(back);
}