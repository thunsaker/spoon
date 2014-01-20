#include <pebble.h>
#include "checkinresult.h"
	
static Window *window;
static TextLayer *text_layer;
static BitmapLayer *image_layer_check;
static GBitmap *image_check_big;
	
void checkinresult_init(void){
	window = window_create();
	
	Layer *window_layer = window_get_root_layer(window);

	GRect bounds = layer_get_frame(window_layer);

	image_check_big = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHECK_GRANDE);
	image_layer_check = bitmap_layer_create(GRect(0,10,bounds.size.w, 38));
	bitmap_layer_set_alignment(image_layer_check, GAlignCenter);
	bitmap_layer_set_bitmap(image_layer_check, image_check_big);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer_check));

	text_layer = text_layer_create(GRect(0,52, bounds.size.w, 152-42));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

void checkinresult_show(int result, char venue_name[512]){
	window_stack_push(window, true);
	vibes_short_pulse();
	if(result == 1) {		
		static char checkin_result_text[512];
		snprintf(checkin_result_text, sizeof(checkin_result_text), "Successfully checked into %s", venue_name);
		text_layer_set_text(text_layer, checkin_result_text);
	} else {
		text_layer_set_text(text_layer, "There was a problem. :( Please try again.");
	}
}

void checkinresult_destroy(void){
}