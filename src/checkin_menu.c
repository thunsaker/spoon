// 2016 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "libs/pebble-assist.h"
#include "checkin_menu.h"
#include "checkin.h"
#include "colors.h"
#include "share_menu.h"
#include <localize.h>
	
#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 3

static char venueid[128];
static char venuename[512];

static Window *s_window;
static MenuLayer *s_menu_layer;
static Layer *layer_bar;
static bool split_bar;

void checkin_menu_draw_layer_bar(Layer *cell_layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, (GColor)get_primary_color());
	graphics_fill_rect(ctx, GRect(0,0,15,168), 8, GCornerNone);
	
	if(split_bar) {
		graphics_context_set_fill_color(ctx, (GColor)get_back_color());
		graphics_fill_rect(ctx, GRect(0,84,15,84), 8, GCornerNone);
	}
	
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, GPoint(7,10), 3);
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return NUM_MENU_ITEMS;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
	menu_cell_basic_header_draw(ctx, cell_layer, venuename);
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  	switch (cell_index->section) {
    	default:
			switch (cell_index->row) {
			  	case 0:
				  	menu_cell_basic_draw(ctx, cell_layer, _("Checkin"), NULL, NULL);
				  	break;
			  	case 1:
					menu_cell_basic_draw(ctx, cell_layer, _("Share"), NULL, NULL);
					GRect bounds = layer_get_bounds(cell_layer);
					char* alligators = ">>";
					GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
					graphics_draw_text(ctx, alligators, font,
									   GRect(bounds.size.w - 35,
											 (bounds.size.h/2) - 18,
											 30, bounds.size.h),
									   GTextOverflowModeWordWrap, 
									   GTextAlignmentRight, 
									   NULL);
					break;
			  	case 2: 
				  	menu_cell_basic_draw(ctx, cell_layer, _("Private Checkin"), NULL, NULL);
				  	break;
			}
			break;
	}
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	switch (cell_index->row) {
    	case 0:
			vibes_double_pulse();
	  		checkin_send_request(venueid, venuename, 0, 0, 0, true);
      		break;
		case 1:
			share_menu_show(!split_bar, venueid, venuename);
			break;
		case 2:
			vibes_double_pulse();
			checkin_send_request(venueid, venuename, 1, 0, 0, true);
			break;
	}
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	#if PBL_COLOR
		window_set_background_color(window, GColorBlack);
	#else
		window_set_background_color(window, GColorWhite);
	#endif

	#if PBL_COLOR
		// Add offset for left partial menu
		s_menu_layer = menu_layer_create(
			GRect(20,0,bounds.size.w - 20, bounds.size.h - 20));
	#else
		s_menu_layer = menu_layer_create(
			GRect(0,0,bounds.size.w, bounds.size.h));
	#endif
		
	menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback
	});
	
	menu_layer_set_click_config_onto_window(s_menu_layer, window);
	#ifdef PBL_SDK_3
		#ifdef PBL_COLOR
			menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorLightGray);
			menu_layer_set_highlight_colors(s_menu_layer, GColorBlack, (GColor)get_primary_color());
		#else
			menu_layer_set_normal_colors(s_menu_layer, GColorWhite, GColorBlack);
			menu_layer_set_highlight_colors(s_menu_layer, GColorBlack, GColorWhite);
		#endif
	#endif
	layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

	#if PBL_COLOR
		layer_bar = layer_create(GRect(0,0,15,bounds.size.h));
		layer_set_update_proc(layer_bar, checkin_menu_draw_layer_bar);
		layer_add_child(window_layer, layer_bar);
	#endif
}

static void window_unload(Window *window) {
	menu_layer_destroy_safe(s_menu_layer);
}

static void init(void) {
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(s_window, true);	
}

void checkin_menu_deinit(void) {
	window_destroy_safe(s_window);
	share_menu_deinit();
}

void checkin_menu_show(bool menu_mode, char venue_guid[128], char venue_name[512]) {
	split_bar = !menu_mode;
	init();
	strncpy(venueid, venue_guid, sizeof(venueid));
	strncpy(venuename, venue_name, sizeof(venuename));
}

bool checkin_menu_is_on_top() {
	return s_window == window_stack_get_top_window();
}