// 2013 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "venuelist.h"
#include "pebble-assist.h"
#include "common.h"
	
#define MAX_VENUES 10
	
static Window *window;

static TextLayer *text_layer;
static BitmapLayer *image_layer;
static GBitmap *image_spoon;
static ActionBarLayer *action_bar;
static SimpleMenuLayer *venue_menu_layer;
static SimpleMenuSection menu_sections[1];
static SimpleMenuItem menu_items[MAX_VENUES];
static bool switchTip = true;

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

	dict_write_tuplet(iter, &refresh_tuple);
	dict_write_end(iter);
	app_message_outbox_send();
}

void tap_handler(AccelAxisType axis, int32_t direction) {
	vibes_double_pulse();
	getListOfLocations();
}

void enableRefresh() {
	accel_tap_service_subscribe(tap_handler);
}

void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *text_tuple_token = dict_find(iter, SPOON_TOKEN);
	Tuple *text_tuple_latlng = dict_find(iter, SPOON_LOCATION);

	if(text_tuple_token && !text_tuple_latlng) {
		text_layer_set_text(text_layer, "Connected to Foursquare!");
		persist_write_string(TOKEN_KEY, text_tuple_token->value->cstring);

		getListOfLocations();
	} else if(!text_tuple_token) {
		if(!venuelist_is_on_top()) {
			window_stack_pop_all(true);
			venuelist_show();
		}
		Tuple *text_tuple_id = dict_find(iter, SPOON_ID);
		Tuple *text_tuple_name = dict_find(iter, SPOON_NAME);
		Tuple *text_tuple_address = dict_find(iter, SPOON_ADDRESS);

		enableRefresh();
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

	const uint32_t inbound_size = 2048;
	const uint32_t outbound_size = 64;
	app_message_open(inbound_size, outbound_size);
	
	venuelist_init();
}

int main(void) {
	init();
	window = window_create();
	window_stack_push(window, true);

	Layer *window_layer = window_get_root_layer(window);

	GRect bounds = layer_get_frame(window_layer);

	image_spoon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_APP_LOGO_LONG);
	image_layer = bitmap_layer_create(GRect(0,10,bounds.size.w, 20));
	bitmap_layer_set_bitmap(image_layer, image_spoon);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));

	text_layer = text_layer_create(GRect(0,30, bounds.size.w, bounds.size.h));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text(text_layer, "Welcome to Spoon!");
	layer_add_child(window_layer, text_layer_get_layer(text_layer));
	
	if(switchTip) {
		switchTip = false;
		text_layer_set_text(text_layer, "Getting nearest venues. \n\nTip: Shake to refresh");
	} else {
		switchTip = true;
		text_layer_set_text(text_layer, "Getting nearest venues. \n\nTip: Long-press venue");
	}

	if(persist_exists(TOKEN_KEY)) {
		getListOfLocations();
	} else {
		text_layer_set_text(text_layer, "Open the Pebble app on your phone and connect to Foursquare.");
	}

	app_event_loop();

	text_layer_destroy(text_layer);
	window_destroy(window);
}