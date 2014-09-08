#include "venuelistnew.h"
#include <pebble.h>

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_image_check_ok;
static TextLayer *s_textlayer_1;
static BitmapLayer *s_bitmaplayer_1;
static InverterLayer *s_inverterlayer_1;
static MenuLayer *s_menulayer_1;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, true);
  
  s_res_image_check_ok = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHECK_OK);
  // s_textlayer_1
  s_textlayer_1 = text_layer_create(GRect(22, 2, 121, 16));
  text_layer_set_text(s_textlayer_1, "Text layer");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_1);
  
  // s_bitmaplayer_1
  s_bitmaplayer_1 = bitmap_layer_create(GRect(2, 2, 16, 16));
  bitmap_layer_set_bitmap(s_bitmaplayer_1, s_res_image_check_ok);
  bitmap_layer_set_background_color(s_bitmaplayer_1, GColorWhite);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_1);
  
  // s_inverterlayer_1
  s_inverterlayer_1 = inverter_layer_create(GRect(0, 20, 144, 4));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_inverterlayer_1);
  
  // s_menulayer_1
  s_menulayer_1 = menu_layer_create(GRect(0, 24, 144, 144));
  menu_layer_set_click_config_onto_window(s_menulayer_1, s_window);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_menulayer_1);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_1);
  bitmap_layer_destroy(s_bitmaplayer_1);
  inverter_layer_destroy(s_inverterlayer_1);
  menu_layer_destroy(s_menulayer_1);
  gbitmap_destroy(s_res_image_check_ok);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_venuelistnew(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_venuelistnew(void) {
  window_stack_remove(s_window, true);
}
