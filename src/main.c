// 2014 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "venuelist.h"
#include "pebble-assist.h"
#include "common.h"
#include "checkinresult.h"
#include "strap/strap.h"
#include "splash.h"
	
#define KEY_TOKEN 10
	
static Window *window;

static TextLayer *text_layer;
static BitmapLayer *image_layer;
static GBitmap *image_spoon;
static BitmapLayer *image_layer_cog;
static GBitmap *image_cog;
static bool wasOnTop = false;
AppTimer *timer;
static bool canHideSplash = false;

void showConnectedMessage() {
	text_layer_set_text(text_layer, "Connected to Foursquare!");
}

void out_sent_handler(DictionaryIterator *sent, void *context) { }


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) { }

void getListOfLocations() {
	Tuplet refresh_tuple = TupletInteger(SPOON_REFRESH, 1);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		return;
	}

	Layer *window_layer = window_get_root_layer(window);
	layer_remove_from_parent(bitmap_layer_get_layer(image_layer_cog));
	
	image_cog = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOURSQUARE_COG_0);
	image_layer_cog = bitmap_layer_create(GRect(64,82,16,16));
	bitmap_layer_set_bitmap(image_layer_cog, image_cog);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer_cog));
	
	text_layer_set_text(text_layer, "Getting nearest venues. \n\nTip: Long-press for quick check-in.");

	dict_write_tuplet(iter, &refresh_tuple);
	dict_write_end(iter);
	app_message_outbox_send();
}

void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *text_tuple_token = dict_find(iter, SPOON_TOKEN);
	Tuple *text_tuple_latlng = dict_find(iter, SPOON_LOCATION);
	Tuple *text_tuple_result = dict_find(iter, SPOON_RESULT);
	Tuple *text_tuple_name = dict_find(iter, SPOON_NAME);
	Tuple *text_tuple_error = dict_find(iter, SPOON_ERROR);

	if(text_tuple_error) {
		text_layer_set_text(text_layer, text_tuple_error->value->cstring);
	} else if(text_tuple_token && !text_tuple_latlng) {
		text_layer_set_text(text_layer, "Connected to Foursquare!");
		persist_write_string(KEY_TOKEN, text_tuple_token->value->cstring);

		getListOfLocations();
	} else if(text_tuple_result) {
		checkinresult_show(text_tuple_result->value->int16, text_tuple_name->value->cstring);
	} else if(!text_tuple_token) {
		bool listOnTop = venuelist_is_on_top();
		if(!listOnTop && !wasOnTop && canHideSplash) {
			window_stack_pop_all(true);
			venuelist_show();
			wasOnTop = true;
		}
		
		if (listOnTop || (!listOnTop && !canHideSplash)) {
			venuelist_in_received_handler(iter);
		} else {
			app_message_outbox_send();
		}
	} else if(text_tuple_latlng) {
		text_layer_set_text(text_layer, text_tuple_latlng->value->cstring);
	} else {
		if(!text_tuple_token) {
			text_layer_set_text(text_layer, "Please connect to Foursquare");
		} else {
			text_layer_set_text(text_layer, "Cannot determine current location. :(");
		}
	}
}

char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

void in_dropped_handler(AppMessageResult reason, void *context) {
   	APP_LOG(APP_LOG_LEVEL_DEBUG, "In dropped: %i - %s", reason, translate_error(reason));
}

void load_welcome_window() {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	image_spoon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_APP_LOGO_LONG);
	image_layer = bitmap_layer_create(GRect(0,5,bounds.size.w, 20));
	bitmap_layer_set_bitmap(image_layer, image_spoon);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));

	text_layer = text_layer_create(GRect(0,25, bounds.size.w, bounds.size.h));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text(text_layer, "Welcome to Spoon!");
	layer_add_child(window_layer, text_layer_get_layer(text_layer));
	
	image_cog = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOURSQUARE_COG_0);
	image_layer_cog = bitmap_layer_create(GRect(64,130,16,16));
	bitmap_layer_set_bitmap(image_layer_cog, image_cog);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer_cog));
}
		   
void attempt_load_venues() {
	if(bluetooth_connection_service_peek()) {
		if(persist_exists(KEY_TOKEN)) {
			getListOfLocations();
		} else {
			text_layer_set_text(text_layer, "Connect to Foursquare using the Pebble app on your phone.");
		}
	} else {
		text_layer_set_text(text_layer, "Error:\nCan't load venues, no connection to phone.");
		layer_remove_from_parent(bitmap_layer_get_layer(image_layer_cog));
	}
}

void splash_callback(void *data) {
	canHideSplash = true;
}

void window_load() {
	window = window_create();
	window_stack_push(window, true);
	show_splash();
	load_welcome_window();
	attempt_load_venues();
	timer = app_timer_register(2000, (AppTimerCallback)splash_callback, NULL);
}

void init() {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);	
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	window_load();
	venuelist_init();
	checkinresult_init();
	strap_init(); // initialize strap!
}

void window_unload() {
	text_layer_destroy(text_layer);
	window_destroy(window);
}

void deinit() {
	venuelist_destroy();
	checkinresult_destroy();
	strap_deinit();
	window_unload();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}