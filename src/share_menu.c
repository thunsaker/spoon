// 2016 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "libs/pebble-assist.h"
#include "share_menu.h"
#include "checkin.h"
#include "colors.h"
#include <localize.h>

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 3
#define BOX_SIZE 13

#ifdef PBL_ROUND
	#define SIDE_BAR_WIDTH 40
#else
	#define SIDE_BAR_WIDTH 15
#endif

static char venueid[128];
static char venuename[512];

static Window *s_window;
static MenuLayer *s_menu_layer;
#ifdef PBL_COLOR
	static Layer *layer_bar;
#endif
static bool split_bar;

static GBitmap *check_bitmap;

static bool twitter;
static bool facebook;

#ifdef PBL_COLOR
void share_menu_draw_layer_bar(Layer *cell_layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, (GColor)get_primary_color());
	graphics_fill_rect(ctx, GRect(0,0,SIDE_BAR_WIDTH,168), 8, GCornerNone);

	if(split_bar) {
		Layer *window_layer = window_get_root_layer(s_window);
		GRect bounds = layer_get_frame(window_layer);
		graphics_context_set_fill_color(ctx, (GColor)get_back_color());
		graphics_fill_rect(ctx, GRect(0,bounds.size.h/2,SIDE_BAR_WIDTH,bounds.size.h/2), 8, GCornerNone);
	}

	#ifdef PBL_ROUND
	#else
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_circle(ctx, GPoint(7,10), 3);

		graphics_fill_circle(ctx, GPoint(7,20), 3);
	#endif
}
#endif

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
	#ifdef PBL_ROUND
		GRect bounds = layer_get_bounds(cell_layer);
		GFont little_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
		graphics_draw_text(ctx, _("Share To..."), little_font,
						   GRect(40, 0, bounds.size.w - 20, 25),
						   GTextOverflowModeTrailingEllipsis,
						   GTextAlignmentLeft,
						   NULL);
	#else
		menu_cell_basic_header_draw(ctx, cell_layer, _("Share To..."));
	#endif
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	return PBL_IF_ROUND_ELSE(50, MENU_CELL_BASIC_CELL_HEIGHT);
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	GRect bounds = layer_get_bounds(cell_layer);
	GRect box_bounds = GRect(bounds.size.w - 25,
								(bounds.size.h / 2) - (BOX_SIZE / 2),
								BOX_SIZE, BOX_SIZE);
	GBitmap *check = check_bitmap;
	GRect bitmap_bounds = gbitmap_get_bounds(check);
	GRect check_bounds = GRect(bounds.size.w - 22,
								(bounds.size.h / 2) - (bitmap_bounds.size.h /2) - 2,
								bitmap_bounds.size.w, bitmap_bounds.size.h);

  	switch (cell_index->section) {
    	default:
			graphics_context_set_stroke_color(ctx, GColorWhite);
			switch (cell_index->row) {
			  	case 0:
				  	menu_cell_basic_draw(ctx, cell_layer, _("Twitter"), NULL, NULL);
					graphics_draw_rect(ctx, box_bounds);
					if(twitter) {
						graphics_context_set_compositing_mode(ctx, GCompOpSet);
						graphics_draw_bitmap_in_rect(ctx, check, check_bounds);
					}
				  	break;
			  	case 1:
					menu_cell_basic_draw(ctx, cell_layer, _("Facebook"), NULL, NULL);
					graphics_draw_rect(ctx, box_bounds);
					if(facebook) {
						graphics_context_set_compositing_mode(ctx, GCompOpSet);
						graphics_draw_bitmap_in_rect(ctx, check, check_bounds);
					}
					break;
			  	case 2:
				  	menu_cell_basic_draw(ctx, cell_layer, _("Checkin"), NULL, NULL);
				  	break;
			}
			break;
	}
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	switch (cell_index->row) {
    	case 0:
			twitter = !twitter;
			menu_layer_reload_data(menu_layer);
      		break;
		case 1:
			facebook = !facebook;
			menu_layer_reload_data(menu_layer);
			break;
		case 2:
			vibes_double_pulse();
			checkin_send_request(venueid, venuename, 0, twitter ? 1 : 0, facebook ? 1 : 0, true);
			break;
	}
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	#ifdef PBL_COLOR
		window_set_background_color(window, GColorBlack);
	#else
		window_set_background_color(window, GColorWhite);
	#endif

	#ifdef PBL_COLOR
		#ifdef PBL_ROUND
			s_menu_layer = menu_layer_create(
				GRect(20,0,bounds.size.w - 40, bounds.size.h));
		#else
			s_menu_layer = menu_layer_create(
				GRect(20,0,bounds.size.w - 20, bounds.size.h - 20));
		#endif
	#else
		s_menu_layer = menu_layer_create(
			GRect(0,0,bounds.size.w, bounds.size.h));
	#endif

	menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback
	});

	menu_layer_set_click_config_onto_window(s_menu_layer, window);
	#ifdef PBL_COLOR
		menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorLightGray);
		menu_layer_set_highlight_colors(s_menu_layer, GColorBlack, (GColor)get_primary_color());
	#else
		menu_layer_set_normal_colors(s_menu_layer, GColorWhite, GColorBlack);
		menu_layer_set_highlight_colors(s_menu_layer, GColorBlack, GColorWhite);
	#endif

	layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

	#if PBL_COLOR
		layer_bar = layer_create(GRect(0,0,SIDE_BAR_WIDTH,bounds.size.h));
		layer_set_update_proc(layer_bar, share_menu_draw_layer_bar);
		layer_add_child(window_layer, layer_bar);
	#endif

	check_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHECK_ON);
}

static void window_unload(Window *window) {
	menu_layer_destroy_safe(s_menu_layer);
	gbitmap_destroy(check_bitmap);
}

static void init(void) {
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(s_window, true);
}

void share_menu_deinit(void) {
	window_destroy_safe(s_window);
}

void share_menu_show(bool menu_mode, char venue_guid[128], char venue_name[512]) {
	split_bar = !menu_mode;
	init();
	strncpy(venueid, venue_guid, sizeof(venueid));
	strncpy(venuename, venue_name, sizeof(venuename));
}

/*
bool share_menu_is_on_top() {
	return s_window == window_stack_get_top_window();
}
*/
