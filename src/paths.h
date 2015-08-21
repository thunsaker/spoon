#pragma once
#include <pebble.h>
	
// Conversion to GPath from SVG http://ardnejar.github.io/GPath.svg/
// Icon Source: https://materialdesignicons.com/ and https://www.google.com/design/icons/

static const GPathInfo CHECK_PATH_POINTS = {
	7,
    (GPoint []) {
      {24, 5},
      {8, 21},
      {0, 14},
      {2, 12},
      {8, 17},
      {22, 3},
      {24, 5}
    }
};

// TODO: Round this out later
static const GPathInfo REFRESH_PATH_POINTS = {
    14,
    (GPoint []) {
      {20, 4},
      {12, 0}, // CURVE CONVERTED
      {0, 12}, // CURVE CONVERTED
      {12, 24}, // CURVE CONVERTED
      {24, 15}, // CURVE CONVERTED
      {20, 15},
      {12, 21}, // CURVE CONVERTED
      {3, 12}, // CURVE CONVERTED
      {12, 3}, // CURVE CONVERTED
      {18, 6}, // CURVE CONVERTED
      {14, 11},
      {24, 11},
      {24, 0},
      {20, 4}
    }
};