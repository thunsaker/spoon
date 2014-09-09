// 2014 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "venuelist.h"
#include "pebble-assist.h"
#include "common.h"
#include "checkinresult.h"
	
#define MAX_VENUES 10
#define KEY_TOKEN 10
	
static Window *window;

static TextLayer *text_layer;
static BitmapLayer *image_layer;
static GBitmap *image_spoon;
static BitmapLayer *image_layer_cog;
static GBitmap *image_cog;

static void image_layer_update_callback(Layer *layer, GContext *ctx) {
	graphics_draw_bitmap_in_rect(ctx, image_spoon, layer_get_bounds(layer));
}

void showConnectedMessage() {
	text_layer_set_text(text_layer, "Connected to Foursquare!");
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
// outgoing message was delivered
}


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
// outgoing message failed
}

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

void tap_handler(AccelAxisType axis, int32_t direction) {
	// TODO: Do nothing, thinking about making this a "double-tap" so it doesn't trigger a reload when you try to turn on the backlight.
	/*
	vibes_double_pulse();
	getListOfLocations();
	*/
}

void enableRefresh() {
	accel_tap_service_subscribe(tap_handler);
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
		if(!venuelist_is_on_top()) {
			window_stack_pop_all(true);
			venuelist_show();
		}
		//enableRefresh();
		if (venuelist_is_on_top()) {
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

void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);	
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	venuelist_init();
	checkinresult_init();
}

int main(void) {
	init();
	window = window_create();
	window_stack_push(window, true);

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

	app_event_loop();

	text_layer_destroy(text_layer);
	window_destroy(window);
}