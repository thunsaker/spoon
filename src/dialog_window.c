// 2015 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "libs/pebble-assist.h"
#include "dialog_window.h"

static Window *s_dialog_window;
static TextLayer *s_label_layer;
static BitmapLayer *s_icon_layer;
static int id;

static GBitmap *s_icon_bitmap;

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WARNING);
	GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

	s_icon_layer = bitmap_layer_create(GRect(10, 10, bitmap_bounds.size.w, bitmap_bounds.size.h));
	bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
	bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

	s_label_layer = text_layer_create(GRect(10, 10 + bitmap_bounds.size.h + 5, 124, 168 - (10 + bitmap_bounds.size.h + 10)));
	switch(id) {
		case 0: // No Foursquare Connection
			text_layer_set_text(s_label_layer, DIALOG_MESSAGE_NOT_CONNECTED);
			break;
		case 1: // No Phone Connection
			text_layer_set_text(s_label_layer, DIALOG_MESSAGE_NO_PHONE);
			break;
		case 2: // No location
			text_layer_set_text(s_label_layer, DIALOG_MESSAGE_NO_LOCATION);
			break;
	}
	text_layer_set_background_color(s_label_layer, GColorClear);
	text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(s_label_layer));
}

static void window_unload(Window *window) {
	//menu_layer_destroy_safe(s_menu_layer);
}

static void init(void) {
	s_dialog_window = window_create();
	window_set_window_handlers(s_dialog_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(s_dialog_window, true);	
}

static void deinit(void) {
	window_destroy_safe(s_dialog_window);
}

void dialog_window_show(int message_id) {
	id = message_id;
	init();
}