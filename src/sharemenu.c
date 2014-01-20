// 2014 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "pebble-assist.h"
#include "sharemenu.h"
#include "checkin.h"
	
static Window *share_menu_window;
static MenuLayer *share_menu_layer;
static GBitmap *image_share_twitter;
static GBitmap *image_share_facebook;
static GBitmap *image_check_on;
static GBitmap *image_check_off;

static SimpleMenuItem shareMenuItems[3] = {};
static SimpleMenuSection shareMenuSection[1] = {};

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static char venueid[128];
static char venuename[512];
static bool shareTwitter;
static bool shareFacebook;

void sharemenu_show(char venue_guid[128], char venue_name[512]) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Inside sharemenu_show");
	strncpy(venueid, venue_guid, sizeof(venueid));
	strncpy(venuename, venue_name, sizeof(venuename));
	
	share_menu_window = window_create();
	
	shareTwitter = false;
	shareFacebook = false;
	
	image_share_facebook = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARE_FACEBOOK);
	image_share_twitter = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARE_TWITTER);
	
	image_check_on = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHECK_ON);
	image_check_off = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHECK_OFF);
	
	// Create Menu
	shareMenuSection[0].items = shareMenuItems;
	shareMenuSection[0].num_items = 3;
	shareMenuSection[0].title = "Share Options";
	
	shareMenuItems[0].title = "Twitter";
	shareMenuItems[0].icon = image_check_off;
	
	shareMenuItems[1].title = "Facebook";
	shareMenuItems[1].icon = image_check_off;
	
	shareMenuItems[2].title = "Share";
	
	share_menu_layer = menu_layer_create_fullscreen(share_menu_window);
	menu_layer_set_callbacks(share_menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
	});

	menu_layer_set_click_config_onto_window(share_menu_layer, share_menu_window);
	menu_layer_add_to_window(share_menu_layer, share_menu_window);
	menu_layer_set_selected_index(share_menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(share_menu_layer);
	window_stack_push(share_menu_window, true);
}

void sharemenu_init(void) {
}

void sharemenu_destroy(void) {
	layer_remove_from_parent(menu_layer_get_layer(share_menu_layer));
	menu_layer_destroy_safe(share_menu_layer);
	window_destroy_safe(share_menu_window);
}

bool sharemenu_is_on_top() {
	return share_menu_window == window_stack_get_top_window();
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return 3;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	return 45;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, shareMenuSection[section_index].title);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if(cell_index->row == 0 && shareTwitter == true) {
		menu_cell_basic_draw(ctx, cell_layer, shareMenuItems[cell_index->row].title, NULL, image_check_on);
	} else if(cell_index->row == 1 && shareFacebook == true) {
		menu_cell_basic_draw(ctx, cell_layer, shareMenuItems[cell_index->row].title, NULL, image_check_on);	
	} else {
		menu_cell_basic_draw(ctx, cell_layer, shareMenuItems[cell_index->row].title, NULL, shareMenuItems[cell_index->row].icon);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if(cell_index->row == 0) { // Flag Twitter
		if(shareTwitter == false) {
			shareTwitter = true;
		} else {
			shareTwitter = false;
		}
		menu_layer_reload_data_and_mark_dirty(share_menu_layer);
	} else if(cell_index->row == 1) { // Flag Facebook
		if(shareFacebook == false) {
			shareFacebook = true;
		} else {
			shareFacebook = false;
		}
		menu_layer_reload_data_and_mark_dirty(share_menu_layer);
	} else { // Share Item
		int twitter = 0;
		int facebook = 0;
		if(shareTwitter == true) {
			twitter = 1;
		}
		if(shareFacebook == true) {
			facebook = 1;
		}
		vibes_double_pulse();
		send_checkin_request(venueid, venuename, 0, twitter, facebook);
		window_stack_pop(true);
	}
}