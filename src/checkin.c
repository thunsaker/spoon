// 2016 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "checkin.h"
#include "colors.h"
#include "common.h"
#include "libs/pebble-assist.h"
#include "paths.h"

static Window *s_main_window;
#ifdef PBL_SDK_3
	static StatusBarLayer *s_status_bar;
#endif

static Layer *layer_back;
static Layer *layer_check;
static TextLayer *text_layer_status;
#ifdef PBL_SDK_3
	static Layer *layer_countdown_bar;
#endif

static bool hasResult;
static bool checkinResult;

#ifdef PBL_COLOR
	uint8_t back_color;
#endif

AppTimer *pulse_check_timer;
AppTimer *checkin_timeout_timer;
AppTimer *countdown_timer;

static int pulse = 0;
static int progress = 100;

static GPath *s_check_large_path = NULL;

void checkin_show(void);
static void countdown_tick(void *ctx);

void checkin_result_receiver(bool result) {
	hasResult = true;
	checkinResult = result;
}

static void start_countdown() {
	countdown_timer = app_timer_register(CHECKIN_COUNTDOWN_DELTA, countdown_tick, NULL);
}

static void countdown_tick(void *ctx) {
	progress -= (progress > 0) ? 1 : 100;
	#ifdef PBL_SDK_3
		layer_mark_dirty(layer_countdown_bar);
	#endif
	if(progress > 0) {
		start_countdown();
	} else {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Pop it like it's hot!");
// 		#ifdef PBL_SDK_3
			window_stack_pop_all(true);
// 		#else
// 			window_stack_pop(s_main_window);
// 		#endif
	}
}

void checkin_send_request(char venue_guid[128], char venue_name[512], int private, int twitter, int facebook, bool show_checkin) {
	if(venue_guid) {
		Tuplet guid_tuple = TupletCString(SPOON_ID, venue_guid);
		Tuplet name_tuple = TupletCString(SPOON_NAME,  venue_name);
		Tuplet private_tuple = TupletInteger(SPOON_PRIVATE, private);
		Tuplet twitter_tuple = TupletInteger(SPOON_TWITTER, twitter);
		Tuplet facebook_tuple = TupletInteger(SPOON_FACEBOOK, facebook);
		
		DictionaryIterator *iter;
		app_message_outbox_begin(&iter);
		
		if(iter == NULL) {
			return;
		}
		dict_write_tuplet(iter, &guid_tuple);
		dict_write_tuplet(iter, &name_tuple);
		dict_write_tuplet(iter, &private_tuple);
		dict_write_tuplet(iter, &twitter_tuple);
		dict_write_tuplet(iter, &facebook_tuple);
		dict_write_end(iter);
		
		app_message_outbox_send();
		
		if(show_checkin) {
			checkin_show();
		}
	} else {
		return;
	}
}

void pulse_check_tick() {
	if(pulse == 0) {
		pulse = 1;
		s_check_large_path = gpath_create(&CHECK_LARGE_2_PATH_POINTS);
	} else {
		pulse = 0;
		s_check_large_path = gpath_create(&CHECK_LARGE_PATH_POINTS);
	}

	layer_mark_dirty(layer_check);
	if(hasResult) {
		#ifdef PBL_COLOR
			back_color = checkinResult ? GColorIslamicGreen.argb : GColorSunsetOrange.argb;
		#endif
			
		s_check_large_path = gpath_create(&CHECK_LARGE_PATH_POINTS);
		text_layer_set_text(text_layer_status, checkinResult ? "Checked In!" : "Something went wrong :(");
		
		layer_mark_dirty(layer_check);
		app_timer_cancel_safe(pulse_check_timer);
		app_timer_cancel_safe(checkin_timeout_timer);
		start_countdown();
	} else {
		pulse_check_timer = app_timer_register(100, pulse_check_tick, NULL);
	}
}


void checkin_timeout_tick() {
	app_timer_cancel_safe(pulse_check_timer);
	// TODO: Replace with frown?
	//s_check_large_path = gpath_create(&CHECK_LARGE_PATH_POINTS);
	#ifdef PBL_COLOR
		back_color = GColorDarkGray.argb;
	#endif
		
	text_layer_set_text(text_layer_status, "Timeout :(");
	layer_mark_dirty(layer_check);
}

#ifdef PBL_SDK_3
void draw_countdown_bar(Layer *cell_layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(cell_layer);

	int width = (int)(float)(((float)progress / 100.0F) * bounds.size.w);
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, GPointZero, GPoint(width, 0));
}
#endif

void draw_layer_check(Layer *cell_layer, GContext *ctx) {
	graphics_context_set_stroke_color(ctx, GColorBlack);
	gpath_draw_outline(ctx, s_check_large_path);
	#ifdef PBL_SDK_3
		graphics_context_set_stroke_width(ctx, 2);
	#endif
	graphics_context_set_fill_color(ctx, GColorWhite);
	gpath_draw_filled(ctx, s_check_large_path);
}

void draw_layer_back(Layer *cell_layer, GContext *ctx) {
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, (GColor)back_color);
	#else
		graphics_context_set_fill_color(ctx, GColorBlack);
	#endif
	graphics_fill_rect(ctx, GRect(0,0,144,168), 0, GCornerNone);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);

	GRect bounds = layer_get_frame(window_layer);
	#ifdef PBL_COLOR
		back_color = get_accent_color();
	#endif

	// Background
  	#if PBL_COLOR
		window_set_background_color(window, (GColor)back_color);
  	#else
    	window_set_background_color(window, GColorWhite);
  	#endif

	// Animating Background
	layer_back = layer_create(GRect(0,0,bounds.size.w,bounds.size.h));
	layer_set_update_proc(layer_back, draw_layer_back);
	layer_add_child(window_layer, layer_back);

	// Text Status
	text_layer_status = text_layer_create(GRect(10,120 - STATUS_BAR_OFFSET,bounds.size.w-20,40));
	#ifdef PBL_COLOR
		text_layer_set_text_color(text_layer_status, GColorBlack);
	#else
		text_layer_set_text_color(text_layer_status, GColorWhite);
	#endif
	text_layer_set_background_color(text_layer_status, GColorClear);
	text_layer_set_text_alignment(text_layer_status, GTextAlignmentCenter);
	text_layer_set_font(text_layer_status, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	text_layer_set_text(text_layer_status, "Checking in...");
	layer_add_child(window_layer, text_layer_get_layer(text_layer_status));

	// Check
	s_check_large_path = gpath_create(&CHECK_LARGE_PATH_POINTS);
	layer_check = layer_create(GRect(0,0 - STATUS_BAR_OFFSET,bounds.size.w,bounds.size.h));
	layer_set_update_proc(layer_check, draw_layer_check);
	layer_add_child(window_layer, layer_check);

	pulse_check_timer = app_timer_register(100, pulse_check_tick, NULL);
	checkin_timeout_timer = app_timer_register(20000, checkin_timeout_tick, NULL);

	// Status Bar
	#ifdef PBL_SDK_3
		s_status_bar = status_bar_layer_create();
		status_bar_layer_set_separator_mode(
			s_status_bar, StatusBarLayerSeparatorModeDotted);
		status_bar_layer_set_colors(
			s_status_bar, GColorClear, GColorBlack);
		layer_add_child(
			window_layer, status_bar_layer_get_layer(s_status_bar));
		
		layer_countdown_bar = layer_create((GRect) {
			.origin = GPoint(0, STATUS_BAR_LAYER_HEIGHT - 2),
			.size = CHECKIN_COUNTDOWN_BAR_SIZE
		});
		layer_set_update_proc(layer_countdown_bar, draw_countdown_bar);
		layer_add_child(window_layer, layer_countdown_bar);
	#endif
}

static void window_unload(Window *window) {
	app_timer_cancel_safe(countdown_timer);
}

static void init(void) {
	s_main_window = window_create();
	// TODO: Dismiss message with any click
	//window_set_click_config_provider(s_main_window, click_config_provider);
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(s_main_window, true);
}

void checkin_deinit(void) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Checkin deinit");
	layer_destroy_safe(layer_back);
	layer_destroy_safe(layer_check);
	text_layer_destroy_safe(text_layer_status);
	#ifdef PBL_SDK_3
		layer_destroy_safe(layer_countdown_bar);
	#endif
	window_destroy_safe(s_main_window);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Checkin deinit");
}

void checkin_show(void) {
	init();
}