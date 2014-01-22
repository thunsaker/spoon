// 2014 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "pebble-assist.h"
#include "common.h"
#include "venueconfirmation.h"
#include "sharemenu.h"
#include "checkin.h"

static char venueid[128];
static char venuename[512];

static Window *window;
static TextLayer *text_layer_prompt;
static TextLayer *text_layer_name;
static ActionBarLayer* actionBar;
static GBitmap* buttonCheck;
static GBitmap* buttonPrivate;
static GBitmap* buttonShare;

void down_single_click_handler_confirmation(ClickRecognizerRef recognizer, Window *window) {
	vibes_double_pulse();
	send_checkin_request(venueid, venuename, 0, 0, 0);
	window_stack_pop(true);
}

void up_single_click_handler_confirmation(ClickRecognizerRef recognizer, Window *window) {
	vibes_double_pulse();
	send_checkin_request(venueid, venuename, 1, 0, 0);
	window_stack_pop(true);
}

void select_single_click_handler_confirmation(ClickRecognizerRef recognizer, Window *window) {
	vibes_short_pulse();
	window_stack_pop(true);
	sharemenu_show(venueid, venuename);
}

void click_config_confirmation(void *context) {
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_single_click_handler_confirmation);
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_single_click_handler_confirmation);
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_single_click_handler_confirmation);
}

void venueconfirmation_show(char venue_guid[128], char venue_name[512]){
	strncpy(venueid, venue_guid, sizeof(venueid));
	strncpy(venuename, venue_name, sizeof(venuename));
	
	window = window_create();
	window_stack_push(window, true);
	Layer* window_layer = window_get_root_layer(window);
	
	text_layer_prompt = text_layer_create(GRect(5,5, 144 - 30, 40));
	text_layer_set_text_alignment(text_layer_prompt, GTextAlignmentLeft);
	text_layer_set_overflow_mode(text_layer_prompt, GTextOverflowModeWordWrap);
	text_layer_set_font(text_layer_prompt, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text(text_layer_prompt, "Check in?");
	layer_add_child(window_layer, (Layer *)text_layer_prompt);
	
	text_layer_name = text_layer_create(GRect(5,50, 144 - 30, 100));
	text_layer_set_text_alignment(text_layer_name, GTextAlignmentLeft);
	text_layer_set_overflow_mode(text_layer_name, GTextOverflowModeWordWrap);
	text_layer_set_font(text_layer_name, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text(text_layer_name, venuename);
	layer_add_child(window_layer, (Layer *)text_layer_name);
	
	actionBar = action_bar_layer_create();
	action_bar_layer_set_click_config_provider(actionBar, (ClickConfigProvider) click_config_confirmation);
	buttonCheck = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHECK_OK);
	action_bar_layer_set_icon(actionBar, BUTTON_ID_DOWN, buttonCheck);
	buttonPrivate = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHECK_PRIVATE);
	action_bar_layer_set_icon(actionBar, BUTTON_ID_UP, buttonPrivate);
	buttonShare = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHECK_SHARE);
	action_bar_layer_set_icon(actionBar, BUTTON_ID_SELECT, buttonShare);
	action_bar_layer_add_to_window(actionBar, window);
}

void venueconfirmation_init(void) {
}

void venueconfirmation_destroy(void) {
	window_destroy_safe(window);
}

bool venueconfirmation_is_on_top() {
	return window == window_stack_get_top_window();
}