#include "splash.h"
#include <pebble.h>

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_image_app_name;
static GBitmap *s_res_image_app_logo_long_new_white;
static Layer *image_layer_spoon_holder;
static BitmapLayer *image_layer_text;
static RotBitmapLayer *image_layer_spoon_new;
static AppTimer *rotate_timer;

void spinner_callback(void *data) {
	rot_bitmap_layer_increment_angle(image_layer_spoon_new, 15);
}

static void initialise_ui(void) {
	s_window = window_create();
	window_set_background_color(s_window, GColorBlack);
	window_set_fullscreen(s_window, false);

	s_res_image_app_name = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_APP_NAME);
	s_res_image_app_logo_long_new_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_APP_LOGO_LONG_NEW_WHITE);
	
	// image_layer_text
	image_layer_text = bitmap_layer_create(GRect(33, 102, 78, 26));
	bitmap_layer_set_bitmap(image_layer_text, s_res_image_app_name);
	layer_add_child(window_get_root_layer(s_window), (Layer *)image_layer_text);

	// image_layer_spoon_new
	image_layer_spoon_holder = layer_create(GRect(36, 20, 64, 64));
	
	// image_layer_spoon_new
	image_layer_spoon_new = rot_bitmap_layer_create(s_res_image_app_logo_long_new_white);
	layer_add_child(image_layer_spoon_holder, (Layer *)image_layer_spoon_new);
	layer_add_child(window_get_root_layer(s_window), (Layer *)image_layer_spoon_holder);
	
	rotate_timer = 	app_timer_register(100, (AppTimerCallback)spinner_callback, NULL);
}

static void destroy_ui(void) {
	window_destroy(s_window);
	bitmap_layer_destroy(image_layer_text);
	rot_bitmap_layer_destroy(image_layer_spoon_new);
	gbitmap_destroy(s_res_image_app_name);
	gbitmap_destroy(s_res_image_app_logo_long_new_white);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
	destroy_ui();
}

void show_splash(void) {
	initialise_ui();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.unload = handle_window_unload,
	});
	window_stack_push(s_window, true);
}

void hide_splash(void) {
	window_stack_remove(s_window, true);
}