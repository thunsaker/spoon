// 2016 Thomas Hunsaker @thunsaker

#pragma once

#include <pebble.h>

int config_get_theme();
void config_set_theme(int config_theme);
int config_get_unit();
void config_set_unit(int config_unit);
void config_init(int config_theme);