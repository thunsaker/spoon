// 2014 Thomas Hunsaker @thunsaker
// Heavily modified version of Neal's Hacker News list implementation - https://github.com/Neal/pebble-hackernews

#include <pebble.h>
#include "venuelist.h"
#include "pebble-assist.h"
#include "common.h"
#include "venueconfirmation.h"
#include "checkin.h"

#define MAX_VENUES 10

static SpoonVenue venues[MAX_VENUES];

static int num_venues;
static char address[512];
static char error[128];
static char venueid[128];
static char venuename[512];

static void clean_list();
static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static Window *window;
static MenuLayer *menu_layer;

void venuelist_init(void) {
	window = window_create();

	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
		.select_long_click = menu_select_long_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);
}

void venuelist_show() {
	clean_list();
	window_stack_push(window, true);
}

void venuelist_destroy(void) {
	layer_remove_from_parent(menu_layer_get_layer(menu_layer));
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

static void clean_list() {
	memset(venues, 0x0, sizeof(venues));
	num_venues = 0;
	error[0] = '\0';
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

bool venuelist_is_on_top() {
	return window == window_stack_get_top_window();
}

void venuelist_in_received_handler(DictionaryIterator *iter) {
	Tuple *index_tuple = dict_find(iter, SPOON_INDEX);
	Tuple *id_tuple = dict_find(iter, SPOON_ID);
	Tuple *name_tuple = dict_find(iter, SPOON_NAME);
	Tuple *address_tuple = dict_find(iter, SPOON_ADDRESS);
	Tuple *refresh_tuple = dict_find(iter, SPOON_REFRESH);

	if(refresh_tuple) {
		if(refresh_tuple->value->int16 == 1) {
			window_stack_pop_all(true);
			venuelist_destroy();
			venuelist_show();
		}
	}

	if (index_tuple && name_tuple && address_tuple) {
		SpoonVenue venue;
		venue.index = index_tuple->value->int16;
		strncpy(venue.id, id_tuple->value->cstring, sizeof(venue.id));
		strncpy(venue.name, name_tuple->value->cstring, sizeof(venue.name));
		if(address_tuple) {
			strncpy(venue.address, address_tuple->value->cstring, sizeof(venue.address));
		} else {
			strncpy(venue.address, "-", sizeof(venue.address));
		}

		venues[venue.index] = venue;
		num_venues++;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	} else if (name_tuple) {
		strncpy(error, name_tuple->value->cstring, sizeof(error));
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return (num_venues) ? num_venues : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	return MENU_CELL_BASIC_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, "Nearest Venues");
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (strlen(error) != 0) {
		menu_cell_basic_draw(ctx, cell_layer, "Error!", error, NULL);
	} else if (num_venues == 0) {
		menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
	} else {
		menu_cell_basic_draw(ctx, cell_layer, venues[cell_index->row].name, venues[cell_index->row].address, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	strncpy(venueid, venues[cell_index->row].id, sizeof(venueid));
	strncpy(venuename, venues[cell_index->row].name, sizeof(venuename));
	venueconfirmation_show(venueid, venuename);
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	vibes_double_pulse();
	strncpy(venueid, venues[cell_index->row].id, sizeof(venueid));
	strncpy(venuename, venues[cell_index->row].name, sizeof(venuename));
	send_checkin_request(venueid, venuename, 0, 0, 0);
}