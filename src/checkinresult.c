// 2014 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "checkinresult.h"
	
static Window *window;
static TextLayer *text_layer;
static BitmapLayer *image_layer_check;
static GBitmap *image_check_big;
static AppTimer *timer_auto_close = NULL;

void single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	app_timer_cancel(timer_auto_close);
	window_stack_pop(true);
}

void click_config_result(void *context) {
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) single_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) single_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) single_click_handler);
}

void timer_auto_close_callback(void* data) {
	window_stack_pop_all(true);
}

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
	window_set_click_config_provider(window, click_config_result);
}

void checkinresult_show(int result, char venue_name[512]){
	window_stack_push(window, true);
	vibes_short_pulse();
	if(result == 1) {
		// Add facebook/twitter sharing notification
		static char checkin_result_text[512];
		snprintf(checkin_result_text, sizeof(checkin_result_text), "Successfully checked into %s", venue_name);
		text_layer_set_text(text_layer, checkin_result_text);
		
		timer_auto_close = app_timer_register(10000, timer_auto_close_callback, NULL);
	} else {
		text_layer_set_text(text_layer, "There was a problem. :( Please try again.");
	}
}

void checkinresult_destroy(void){
}