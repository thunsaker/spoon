// 2015 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "config.h"
#include "common.h"

static int theme = 0;
static int unit = 0;

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
	unit = unit;
}

// void config_init(int config_theme, int config_unit) {
void config_init(int config_theme) {
	config_set_theme(config_theme);
// 	config_set_unit(config_unit);
}