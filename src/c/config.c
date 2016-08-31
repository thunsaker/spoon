// 2016 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "src/c/config.h"
#include "src/c/common.h"

static int theme = 0;
static int unit = 0;
static bool timeline = false;

int config_get_theme() {
	return persist_read_int(KEY_THEME);
}

void config_set_theme(int config_theme) {
	theme = config_theme;
}

int config_get_unit() {
	return unit;
}

void config_set_unit(int config_unit) {
	unit = config_unit;
}

bool config_get_timeline() {
	return timeline;
}

void config_set_timeline(bool config_timeline) {
	timeline = config_timeline;
}

void config_init(int config_theme) {
	config_set_theme(config_theme);
}