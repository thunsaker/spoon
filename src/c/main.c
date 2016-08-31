// 2016 Thomas Hunsaker @thunsaker

#include <pebble.h>
#include "src/c/libs/pebble-assist.h"
#ifdef PBL_COLOR
	#include "src/c/libs/dithered_rects.h"
	#include "src/c/colors.h"
#endif
#include "src/c/checkin_menu.h"
#include "src/c/checkin.h"
#include "src/c/libs/math-utils.h"
#include "src/c/common.h"
#include "src/c/config.h"
#include "src/c/paths.h"
#include <pebble-localize/pebble-localize.h>

#define BOX_HEIGHT 84
#define ROW_HEIGHT 52

#define ANIM_DURATION 100
#define ANIM_DELAY 300

#define NUM_MENU_SECTIONS 1
#ifdef PBL_PLATFORM_APLITE
	#define NUM_MENU_ITEMS 12
#else
	#define NUM_MENU_ITEMS 17
#endif

#ifdef PBL_PLATFORM_APLITE
	#define MAX_VENUES 10
#else
	#define MAX_VENUES 15
#endif

static SpoonVenue venues[MAX_VENUES];
static int num_venues;
static SpoonVenue lastCheckinVenue;
static char venueid[128];
static char venuename[512];

static int SCREEN_HEIGHT;
static int SCREEN_WIDTH;

static Window *s_main_window;
#ifdef PBL_SDK_3
	static StatusBarLayer *s_status_bar;
#endif

static GBitmap *image_cog;
static BitmapLayer *image_layer_back;
static Layer *layer_back;
static Layer *layer_primary_back;
static TextLayer *text_layer_primary;
static TextLayer *text_layer_primary_address;
static TextLayer *text_layer_primary_distance;
static Layer *layer_primary_circle;
static MenuLayer *layer_menu_venues;
static TextLayer *text_layer_last_checkin_title;
static TextLayer *text_layer_last_checkin_venue;
static TextLayer *text_layer_last_checkin_address;
static TextLayer *text_layer_last_checkin_date;
static Layer *layer_last_checkin;
static GBitmap *image_refresh;

#ifdef PBL_COLOR
	static uint8_t primary_color;
	static uint8_t accent_color;
	static uint8_t back_color;
	static uint8_t new_back_color;
	static uint8_t result_color;
#endif

#define MAX_CIRCLE_RADIUS 142
#define DEFAULT_CIRCLE_RADIUS 21
#define DEFAULT_CIRCLE_RADIUS_MINI 15
#define DEFAULT_CIRCLE_RADIUS_ROUND 80

static bool show_checkin;
static bool grow_circle;
static bool drop_and_shrink;
static int circle_radius = DEFAULT_CIRCLE_RADIUS;
static int circle_radius_count = 1;
AppTimer *circle_grow_timer;
static bool reverse_menu_animation;
static bool menu_mode;
static bool reverse_last_animation;
static bool last_mode;
static bool no_foursquare;
static bool no_internet;
static int up_count = 0;
static bool is_refreshing;

static GPath *s_check_path = NULL;

static Animation *anim_slide_menu;
static Animation *anim_slide_box;
static Animation *anim_slide_circle;
static Animation *anim_slide_text_1;
static Animation *anim_slide_text_2;

static PropertyAnimation *s_drop_current_animation;
static PropertyAnimation *s_drop_last_animation;

static PropertyAnimation *s_transition_box_animation;
static PropertyAnimation *s_transition_text_1_animation;
static PropertyAnimation *s_transition_text_2_animation;
static PropertyAnimation *s_transition_circle_animation;
static PropertyAnimation *s_transition_menu_animation;

const char* locale_current;

const char *getErrorReason(int error_code) {
	switch(error_code) {
		case 0:
			return _("No internet connection detected.");
		break;
		case 1:
			return _("Request timed out!");
		break;
		case 2:
			return _("Error with request :(");
		break;
		case 3:
			return _("Connection Failed. Try Again.");
		break;
		default:
			return _("Something went wrong :(");
		break;
	};
}

const char *get_unit(int unit) {
	switch(unit) {
		case 1:
			return _("km");
		case 2:
			return _("ft");
		case 3:
			return _("mi");
		default:
			return _("m");
	}
}

static void getListOfLocations() {
	is_refreshing = true;
	Tuplet refresh_tuple = TupletInteger(SPOON_REFRESH, MAX_VENUES);
	Tuplet lang_tuple = TupletCString(SPOON_NAME, locale_current);
	
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	if (iter == NULL) {
		return;
	}
	text_layer_set_text(text_layer_primary, _("Refreshing..."));
	text_layer_set_text(text_layer_primary_address, "");

	dict_write_tuplet(iter, &refresh_tuple);
	dict_write_tuplet(iter, &lang_tuple);
	#ifdef PBL_APLITE
		Tuplet platform_tuple = TupletInteger(SPOON_INDEX, 1);
		dict_write_tuplet(iter, &platform_tuple);
	#endif
	dict_write_end(iter);
	app_message_outbox_send();
}

static void transition_circle_anim_stopped_handler(Animation *animation, bool finished, void *ctx) {
	#ifdef PBL_SDK_2
		property_animation_destroy((PropertyAnimation*)s_transition_circle_animation);
	#endif

	layer_mark_dirty(layer_primary_circle);
	if(!last_mode) {
		reverse_menu_animation = false;
	}
}

static void property_anim_stopped_handler(Animation *animation, bool finished, void *ctx) {
	#ifdef PBL_SDK_2
		property_animation_destroy((PropertyAnimation*)animation);
	#endif
}

static void anim_stopped_handler(Animation *animation, bool finished, void *ctx) {
	#ifdef PBL_SDK_2
		animation_destroy(animation);
	#endif
}

static void last_checkin_show() {
	GRect box_start, box_finish;
	GRect last_start, last_finish;
	GRect fab_start, fab_finish;

	box_start = GRect(0, 84 - STATUS_BAR_OFFSET, SCREEN_WIDTH, BOX_HEIGHT);
	box_finish = GRect(0, PBL_IF_ROUND_ELSE(SCREEN_HEIGHT + 5, SCREEN_HEIGHT), SCREEN_WIDTH, BOX_HEIGHT + 10);
	last_start = GRect(0, 0 - SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
	last_finish = GRect(0, 20 - STATUS_BAR_OFFSET, SCREEN_WIDTH, SCREEN_HEIGHT);
	#ifdef PBL_ROUND
		fab_start = GRect(0, 0 - STATUS_BAR_OFFSET, SCREEN_WIDTH, SCREEN_HEIGHT);
		fab_finish =  GRect(0, STATUS_BAR_OFFSET, SCREEN_WIDTH, SCREEN_HEIGHT);
	#else
		fab_start = GRect(0, 0 - STATUS_BAR_OFFSET, SCREEN_WIDTH, SCREEN_HEIGHT);
// 		fab_finish = GRect(SCREEN_WIDTH, 0 - STATUS_BAR_OFFSET, SCREEN_WIDTH, SCREEN_HEIGHT);
		fab_finish = GRect(-94, -45 - STATUS_BAR_OFFSET, SCREEN_WIDTH, SCREEN_HEIGHT);
	#endif

	if(reverse_last_animation) {
		last_mode = false;
	} else {
		last_mode = true;
	}

	// Drop Current Venue
	s_drop_current_animation = reverse_last_animation
		? property_animation_create_layer_frame(layer_primary_back, &box_finish, &box_start)
		: property_animation_create_layer_frame(layer_primary_back, &box_start, &box_finish);
	if(animation_is_scheduled((Animation*)s_drop_current_animation)) {
		animation_unschedule((Animation*)s_drop_current_animation);
	}
	animation_set_duration((Animation*)s_drop_current_animation, ANIM_DURATION);
	animation_set_handlers((Animation*)s_drop_current_animation, (AnimationHandlers) {
		.stopped = property_anim_stopped_handler
	}, NULL);
 	animation_set_delay((Animation*)s_drop_current_animation, ANIM_DELAY);
	animation_set_curve((Animation*)s_drop_current_animation, AnimationCurveEaseOut);
	if(!animation_is_scheduled((Animation*)s_drop_current_animation)) {
		animation_schedule((Animation*)s_drop_current_animation);
	}

	// Drop Last Checkin
	s_drop_last_animation = reverse_last_animation
		? property_animation_create_layer_frame(layer_last_checkin, &last_finish, &last_start)
		: property_animation_create_layer_frame(layer_last_checkin, &last_start, &last_finish);
	animation_set_duration((Animation*)s_drop_last_animation, ANIM_DURATION);
	animation_set_handlers((Animation*)s_drop_last_animation, (AnimationHandlers) {
		.stopped = property_anim_stopped_handler
	}, NULL);
	animation_set_delay((Animation*)s_drop_last_animation, ANIM_DELAY);
	animation_set_curve((Animation*)s_drop_last_animation, AnimationCurveEaseIn);
	if(!animation_is_scheduled((Animation*)s_drop_last_animation)) {
		animation_schedule((Animation*)s_drop_last_animation);
	}
	drop_and_shrink = last_mode;

	// FAB
	s_transition_circle_animation = reverse_last_animation
		? property_animation_create_layer_frame(layer_primary_circle, &fab_finish, &fab_start)
		: property_animation_create_layer_frame(layer_primary_circle, &fab_start, &fab_finish);
	Animation *anim_slide_circle =
		property_animation_get_animation(s_transition_circle_animation);
	animation_set_duration(anim_slide_circle, 500);
	animation_set_handlers((Animation*)s_transition_circle_animation, (AnimationHandlers) {
		.stopped = transition_circle_anim_stopped_handler
	}, NULL);
	if(!animation_is_scheduled((Animation*)anim_slide_circle)) {
		animation_schedule(anim_slide_circle);
	}
}

static void transition_animation() {
	// Primary Back
	GRect start = GRect(0, (SCREEN_HEIGHT/2) - STATUS_BAR_OFFSET, SCREEN_WIDTH, (SCREEN_HEIGHT/2) + STATUS_BAR_OFFSET);
	GRect finish = GRect(0, -10 - ROW_HEIGHT, SCREEN_WIDTH, ROW_HEIGHT + 10);
	if(reverse_menu_animation) {
		s_transition_box_animation =
			property_animation_create_layer_frame(layer_primary_back, &finish, &start);
	} else {
		s_transition_box_animation =
			property_animation_create_layer_frame(layer_primary_back, &start, &finish);
	}

	anim_slide_box =
		property_animation_get_animation(s_transition_box_animation);
	animation_set_duration(anim_slide_box, 500);
	layer_mark_dirty(layer_primary_back);
	animation_set_handlers(anim_slide_box, (AnimationHandlers) {
		.stopped = anim_stopped_handler
	}, NULL);

	// FAB
	start = GRect(0, 0 - STATUS_BAR_OFFSET, SCREEN_WIDTH, SCREEN_HEIGHT);
	finish = GRect(0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
	s_transition_circle_animation = reverse_menu_animation
		? property_animation_create_layer_frame(layer_primary_circle, &finish, &start)
		: property_animation_create_layer_frame(layer_primary_circle, &start, &finish);
	anim_slide_circle =
		property_animation_get_animation(s_transition_circle_animation);
	animation_set_duration(anim_slide_circle, 100);
	animation_set_handlers(anim_slide_circle, (AnimationHandlers) {
		.stopped = anim_stopped_handler
	}, NULL);
	animation_set_handlers((Animation*)s_transition_circle_animation, (AnimationHandlers) {
		.stopped = transition_circle_anim_stopped_handler
	}, NULL);

	// Text 1
	#ifdef PBL_ROUND
		start = GRect(15, 5, SCREEN_WIDTH-25, 50);
		finish = GRect(10, 15, SCREEN_WIDTH, 24);
	#else
		start = GRect(10, 10, 124, 50);
		finish = GRect(10, 15, 102, 24);
	#endif
	if(reverse_menu_animation) {
		text_layer_set_text_color(text_layer_primary, GColorBlack);
		s_transition_text_1_animation =
			property_animation_create_layer_frame(text_layer_get_layer(text_layer_primary), &finish, &start);
	} else {
		#ifdef PBL_COLOR
			text_layer_set_text_color(text_layer_primary, GColorDarkGray);
		#endif
		s_transition_text_1_animation =
			property_animation_create_layer_frame(text_layer_get_layer(text_layer_primary), &start, &finish);
	}
	anim_slide_text_1 =
		property_animation_get_animation(s_transition_text_1_animation);
	animation_set_duration(anim_slide_text_1, 100);
	animation_set_handlers(anim_slide_text_1, (AnimationHandlers) {
		.stopped = anim_stopped_handler
	}, NULL);
	text_layer_set_size(text_layer_primary,GSize(PBL_IF_ROUND_ELSE(SCREEN_WIDTH-25, 102),24));
	text_layer_set_text(text_layer_primary, venues[0].name);

	// Text 2
	#ifdef PBL_ROUND
		start = GRect(30, 50, SCREEN_WIDTH-60, 60);
		finish = GRect(10, 39, 102, 20);
	#else
		start = GRect(10, 60, 124, 20);
		finish = GRect(10, 39, 102, 20);
	#endif
	if(reverse_menu_animation) {
		text_layer_set_text_color(text_layer_primary_address, GColorBlack);
		s_transition_text_2_animation =
			property_animation_create_layer_frame(text_layer_get_layer(text_layer_primary_address), &finish, &start);
	} else {
		#ifdef PBL_COLOR
			text_layer_set_text_color(text_layer_primary_address, GColorDarkGray);
		#endif
		s_transition_text_2_animation =
			property_animation_create_layer_frame(text_layer_get_layer(text_layer_primary_address), &start, &finish);
	}
	anim_slide_text_2 =
		property_animation_get_animation(s_transition_text_2_animation);
	animation_set_handlers(anim_slide_text_2, (AnimationHandlers) {
		.stopped = anim_stopped_handler
	}, NULL);
	animation_set_duration(anim_slide_text_2, 100);

	// TODO: Fix this line, it is crashing
// 	text_layer_set_size(text_layer_primary_address,GSize(102,20));

	// Menu
	start = GRect(0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
	#ifdef PBL_SDK_3
		finish = GRect(0, 15, SCREEN_WIDTH, SCREEN_HEIGHT);
	#else
		finish = GRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	#endif

	if(reverse_menu_animation) {
		s_transition_menu_animation =
			property_animation_create_layer_frame(menu_layer_get_layer(layer_menu_venues), &finish, &start);
		menu_mode = false;
	} else {
		s_transition_menu_animation =
			property_animation_create_layer_frame(menu_layer_get_layer(layer_menu_venues), &start, &finish);
		menu_mode = true;
	}

	anim_slide_menu =
		property_animation_get_animation(s_transition_menu_animation);
	animation_set_duration(anim_slide_menu, 100);
	animation_set_handlers((Animation*)s_transition_menu_animation, (AnimationHandlers) {
		.stopped = property_anim_stopped_handler
	}, NULL);
	animation_set_handlers(anim_slide_menu, (AnimationHandlers) {
		.stopped = anim_stopped_handler
	}, NULL);

	#ifdef PBL_SDK_3
 		Animation *spawn = animation_spawn_create(anim_slide_box, anim_slide_circle, anim_slide_text_1, anim_slide_text_2, anim_slide_menu, NULL);
		animation_schedule(spawn);
	#else
		if(!animation_is_scheduled((Animation*)anim_slide_box)) {
			animation_schedule(anim_slide_box);
		}
		if(!animation_is_scheduled((Animation*)anim_slide_circle)) {
			animation_schedule(anim_slide_circle);
		}
		if(!animation_is_scheduled((Animation*)anim_slide_text_1)) {
			animation_schedule(anim_slide_text_1);
		}
		if(!animation_is_scheduled((Animation*)anim_slide_text_2)) {
			animation_schedule(anim_slide_text_2);
		}
		if(!animation_is_scheduled((Animation*)anim_slide_menu)) {
			animation_schedule(anim_slide_menu);
		}
	#endif
}

void circle_grow_timer_tick() {
	if(circle_radius < MAX_CIRCLE_RADIUS) {
		circle_radius += x_to_the_n(2,circle_radius_count++);
		layer_mark_dirty(layer_primary_circle);
		circle_grow_timer = app_timer_register(100, circle_grow_timer_tick, NULL);
	} else {
		grow_circle = false;
		show_checkin = false;

		circle_radius = DEFAULT_CIRCLE_RADIUS;
		circle_radius_count = 1;

		app_timer_cancel_safe(circle_grow_timer);
		checkin_show();
	}
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(no_foursquare) {
	} else {
		if(menu_mode) {
			up_count = 0;
			if(menu_layer_get_selected_index(layer_menu_venues).row == 1) {
				reverse_menu_animation = true;
				menu_mode = false;
				transition_animation();
			}

			menu_layer_set_selected_next(layer_menu_venues, true, MenuRowAlignCenter, true);
		} else if(last_mode) {
			up_count++;
			if(up_count == 2) {
				getListOfLocations();

				// Return to venue list
				reverse_last_animation = true;
				last_checkin_show();
			}
			// TODO: Add bounce anim
		} else {
			up_count = 0;
			if(strlen(lastCheckinVenue.name) > 0) {
				reverse_last_animation = false;
				last_checkin_show();
			}
		}
	}
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(no_internet || no_foursquare) {
	} else {
		if(menu_mode) {
			menu_layer_set_selected_next(layer_menu_venues, false, MenuRowAlignCenter, true);
		} else if(last_mode) {
			reverse_last_animation = true;
			last_checkin_show();
		} else {
			if(num_venues >= 1) {
				up_count = 0;
				reverse_menu_animation = false;
				transition_animation();
				drop_and_shrink = false;
				menu_layer_set_selected_index(layer_menu_venues, MenuIndex(1,1), MenuRowAlignCenter, true);
			}
		}
	}
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(no_internet || no_foursquare || num_venues == 0) {
	} else {
		if(menu_mode) {
			int selectedIndex = menu_layer_get_selected_index(layer_menu_venues).row;
			if(selectedIndex != NUM_MENU_ITEMS - 1) {
				strncpy(venueid, venues[selectedIndex].id, sizeof(venueid));
				strncpy(venuename, venues[selectedIndex].name, sizeof(venuename));
				checkin_menu_show(menu_mode, venueid, venuename);
			}
		} else if(last_mode) {
			// TODO: Decide IF I want the user to have some action here
			// Do nothing
		} else {
			strncpy(venueid, venues[0].id, sizeof(venueid));
			strncpy(venuename, venues[0].name, sizeof(venuename));
			checkin_menu_show(menu_mode, venueid, venuename);
		}
	}
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(no_internet || no_foursquare || num_venues == 0) {
	} else {
		if(menu_mode) {
			int selectedIndex = menu_layer_get_selected_index(layer_menu_venues).row;
			if(selectedIndex != NUM_MENU_ITEMS - 1) {
				strncpy(venueid, venues[selectedIndex].id, sizeof(venueid));
				strncpy(venuename, venues[selectedIndex].name, sizeof(venuename));
				checkin_send_request(venueid, venuename, 0, 0, 0, true);
				vibes_double_pulse();
			}
		} else {
			strncpy(venueid, venues[0].id, sizeof(venueid));
			strncpy(venuename, venues[0].name, sizeof(venuename));
			show_checkin = true;
			grow_circle = true;

			// Start timer to grow circle
			circle_grow_timer = app_timer_register(100, circle_grow_timer_tick, NULL);
			checkin_send_request(venueid, venuename, 0, 0, 0, false);
			vibes_double_pulse();
		}
	}
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_single_click_handler);
	window_long_click_subscribe(BUTTON_ID_SELECT, 0, NULL, (ClickHandler) select_long_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_single_click_handler);
}

void draw_image_layer_back(Layer *cell_layer, GContext *ctx) {
	GRect bounds = layer_get_frame(cell_layer);
	#ifdef PBL_COLOR
		draw_transitioning_rect(ctx, GRect(0,0,bounds.size.w,168), (GColor)back_color, (GColor)new_back_color);
	#else
		graphics_context_set_fill_color(ctx, GColorDarkGray);
		graphics_fill_rect(ctx, GRect(0,0,bounds.size.w,168/2), 0, GCornersTop);
	#endif
}

void draw_layer_primary_back(Layer *cell_layer, GContext *ctx) {
	GRect bounds = layer_get_frame(cell_layer);
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, (GColor)back_color);
		graphics_fill_rect(ctx, GRect(0,0,bounds.size.w,94), 0, GCornerNone);
	#else
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(0,0,bounds.size.w,2), 0, GCornersTop);
	#endif
}

void draw_layer_primary_circle(Layer *cell_layer, GContext *ctx) {
	if(show_checkin) {
		if(grow_circle) {
			#ifdef PBL_COLOR
				graphics_context_set_fill_color(ctx, (GColor)accent_color);
			#else
				graphics_context_set_fill_color(ctx, GColorBlack);
			#endif

			graphics_fill_circle(ctx, GPoint(113,81), circle_radius);
		}
	} else if(drop_and_shrink) {
		#ifdef PBL_ROUND
		#else
			#ifdef PBL_COLOR
				graphics_context_set_fill_color(ctx, (GColor)accent_color);
				graphics_context_set_stroke_width(ctx, 2);
			#else
				graphics_context_set_fill_color(ctx, GColorBlack);
			#endif

			graphics_fill_circle(ctx, GPoint(113,81), DEFAULT_CIRCLE_RADIUS_MINI);

			graphics_context_set_stroke_color(ctx, GColorWhite);
			graphics_draw_line(ctx, GPoint(105,81), GPoint(111,86));
			graphics_draw_line(ctx, GPoint(111,86), GPoint(120,75));
		#endif
	} else {
		// Shadow
		#ifdef PBL_COLOR
			#ifdef PBL_ROUND
				// Do nothing
			#else
				draw_twenty_percent_circle(ctx, 116, 83, DEFAULT_CIRCLE_RADIUS-1, GColorClear, GColorDarkGray);
			#endif
			graphics_context_set_fill_color(ctx, (GColor)accent_color);
		#else
			graphics_context_set_fill_color(ctx, GColorBlack);
		#endif

		// Circle
		#ifdef PBL_ROUND
			graphics_fill_circle(ctx, GPoint(220,SCREEN_HEIGHT/2), DEFAULT_CIRCLE_RADIUS_ROUND);
		#else
			graphics_fill_circle(ctx, GPoint(113,81), DEFAULT_CIRCLE_RADIUS);
		#endif

		// Icon
		if(is_refreshing) {
			GRect bitmap_bounds = gbitmap_get_bounds(image_refresh);
			#ifdef PBL_ROUND
				GRect refresh_bounds = GRect(SCREEN_WIDTH-32,(SCREEN_HEIGHT/2)-(bitmap_bounds.size.h/2),
											 bitmap_bounds.size.w, bitmap_bounds.size.h);
			#else
				GRect refresh_bounds = GRect(102,71,
											 bitmap_bounds.size.w, bitmap_bounds.size.h);
			#endif
			#ifdef PBL_COLOR
				graphics_context_set_compositing_mode(ctx, GCompOpSet);
			#endif
			graphics_draw_bitmap_in_rect(ctx, image_refresh, refresh_bounds);
		} else {
			graphics_context_set_fill_color(ctx, GColorWhite);
			graphics_context_set_stroke_color(ctx, GColorWhite);
			s_check_path = gpath_create(&CHECK_PATH_POINTS);
			#ifdef PBL_ROUND
				gpath_move_to(s_check_path, GPoint(146,75));
			#else
				gpath_move_to(s_check_path, GPoint(101,70));
			#endif
			gpath_draw_filled(ctx, s_check_path);
		}
	}
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return NUM_MENU_ITEMS;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	return 0;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	return PBL_IF_ROUND_ELSE(MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT, 52);
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) { }

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	char addressString[75];
	snprintf(addressString, sizeof(addressString), 
			 "%s %s - ", venues[cell_index->row].distance, get_unit(venues[cell_index->row].distance_unit));
	size_t len = strlen(addressString);
 	strncat(addressString, venues[cell_index->row].address, 99 - len);
	#ifdef PBL_PLATFORM_APLITE
		if(cell_index->row == NUM_MENU_ITEMS - 1) {
			menu_cell_basic_draw(ctx, cell_layer, _("Foursquare"), _("Powered"), image_cog);
		} else if(strlen(venues[cell_index->row].name) > 0) {
			menu_cell_basic_draw(ctx, cell_layer, venues[cell_index->row].name, addressString, NULL);
		}
	#else
		GRect bounds = layer_get_bounds(cell_layer);
		GFont little_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
		GFont big_font;
		
		// If Russian switch from Roboto to Gothic which has some of the Cyrillic char set
		if (strncmp(locale_current, "ru", 2) == 0) {
			big_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
		} else {
			big_font = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
		}
		if(cell_index->row == NUM_MENU_ITEMS - 1) {
			GRect bitmap_bounds = gbitmap_get_bounds(image_cog);
			#ifdef PBL_ROUND
				graphics_draw_text(ctx, _("Powered by Foursquare"), little_font,
								   GRect(10, (bounds.size.h/2) - 15, bounds.size.w - 20, 25),
								   GTextOverflowModeTrailingEllipsis,
								   GTextAlignmentCenter,
								   NULL);
				GRect cog_bounds = GRect((bounds.size.w / 2) - (bitmap_bounds.size.w / 2),
										 (bounds.size.h/2) + 5,
										 bitmap_bounds.size.w, bitmap_bounds.size.h);
			#else
				graphics_draw_text(ctx, _("Powered by Foursquare"), little_font,
								   GRect(5, 4, bounds.size.w - 10, 20),
								   GTextOverflowModeFill,
								   GTextAlignmentCenter,
								   NULL);
				GRect cog_bounds = GRect((bounds.size.w / 2) - (bitmap_bounds.size.w / 2),
										 30,
										 bitmap_bounds.size.w, bitmap_bounds.size.h);
			#endif
			graphics_context_set_compositing_mode(ctx, GCompOpSet);
			graphics_draw_bitmap_in_rect(ctx, image_cog, cog_bounds);
		} else if (strlen(venues[cell_index->row].name) > 0) {
			#ifdef PBL_ROUND
				GSize text_size = graphics_text_layout_get_content_size(
					venues[cell_index->row].name,
					big_font,
					GRect(0, 5, bounds.size.w - 20, 50),
					GTextOverflowModeTrailingEllipsis,
					GTextAlignmentCenter);
				graphics_draw_text(ctx, venues[cell_index->row].name, big_font,
								   GRect(10, text_size.h > 25 ? 7 : 20, bounds.size.w - 20, 50),
								   GTextOverflowModeTrailingEllipsis,
								   GTextAlignmentCenter,
								   NULL);
				graphics_draw_text(ctx, addressString, little_font,
								   GRect(10, bounds.size.h-24, bounds.size.w - 20, 20),
								   GTextOverflowModeTrailingEllipsis,
								   GTextAlignmentCenter,
								   NULL);
			#else
				graphics_draw_text(ctx, venues[cell_index->row].name, big_font,
								   GRect(5, 4, bounds.size.w - 10, 25),
								   GTextOverflowModeTrailingEllipsis,
								   GTextAlignmentLeft,
								   NULL);
				graphics_draw_text(ctx, addressString, little_font,
								   GRect(5, 30, bounds.size.w - 10, 20),
								   GTextOverflowModeTrailingEllipsis,
								   GTextAlignmentLeft,
								   NULL);
			#endif
		}
	#endif
}

#ifdef PBL_COLOR
static void setup_theme_colors(int theme_id) {
	switch(theme_id) {
		case 1:
		colors_init(GColorOrange.argb, GColorMalachite.argb, GColorWhite.argb);
		break;
		case 2:
		colors_init(GColorFolly.argb, GColorVividCerulean.argb, GColorWhite.argb);
		break;
		case 3:
		colors_init(GColorYellow.argb, GColorIndigo.argb, GColorWhite.argb);
		break;
		case 4:
		colors_init(GColorTiffanyBlue.argb, GColorOrange.argb, GColorWhite.argb);
		break;
		case 5:
		colors_init(GColorDarkGray.argb, GColorBlack.argb, GColorWhite.argb);
		break;
		default:
		colors_init(GColorJaegerGreen.argb, GColorFolly.argb, GColorWhite.argb);
		break;
	}
	window_set_background_color(s_main_window, (GColor)back_color);
}
#endif

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);

	GRect bounds = layer_get_frame(window_layer);
	SCREEN_HEIGHT = bounds.size.h;
	SCREEN_WIDTH = bounds.size.w;

	#ifdef PBL_COLOR
		setup_theme_colors(config_get_theme());
	#endif

	// Background
  	#ifdef PBL_COLOR
		accent_color = get_accent_color();
		back_color = get_back_color();
		primary_color = get_primary_color();
		new_back_color = primary_color;
		result_color = GColorIslamicGreen.argb;
		window_set_background_color(window, (GColor)back_color);
	#else
		window_set_background_color(window, GColorWhite);
  	#endif

	// Main Background
	layer_back = layer_create(GRect(0,0,bounds.size.w,bounds.size.h));
	layer_set_update_proc(layer_back, draw_image_layer_back);
	layer_add_child(window_layer, layer_back);

	#ifdef PBL_COLOR
		// TODO: Show image from the first venue, maybe
		start_transitioning_rect(layer_back, 100, 1);
	#endif

	// Last Checkin
	layer_last_checkin = layer_create(GRect(0,0-bounds.size.h, bounds.size.w, bounds.size.h));

	// Title
	#ifdef PBL_ROUND
		text_layer_last_checkin_title = text_layer_create(GRect(0,7,SCREEN_WIDTH,20));
	#else
	 	text_layer_last_checkin_title = text_layer_create(GRect(40,7,bounds.size.w,20));
	#endif
	text_layer_set_text_color(text_layer_last_checkin_title, GColorBlack);
	text_layer_set_background_color(text_layer_last_checkin_title, GColorClear);
	text_layer_set_text_alignment(text_layer_last_checkin_title, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
	text_layer_set_font(text_layer_last_checkin_title, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	text_layer_set_text(text_layer_last_checkin_title, _("Last Check-In"));
	layer_add_child(layer_last_checkin, text_layer_get_layer(text_layer_last_checkin_title));

	// Venue
	#ifdef PBL_ROUND
		text_layer_last_checkin_venue = text_layer_create(GRect(0,(bounds.size.h/2)-40,SCREEN_WIDTH,74));
	#else
	 	text_layer_last_checkin_venue = text_layer_create(GRect(10,40,bounds.size.w-10,74));
	#endif
	text_layer_set_text_color(text_layer_last_checkin_venue, GColorBlack);
	text_layer_set_background_color(text_layer_last_checkin_venue, GColorClear);
	text_layer_set_text_alignment(text_layer_last_checkin_venue, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
	if (strncmp(locale_current, "ru", 2) == 0) {
		text_layer_set_font(text_layer_last_checkin_venue, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	} else {
		text_layer_set_font(text_layer_last_checkin_venue, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	}
	text_layer_set_text(text_layer_last_checkin_venue, _("Venue Name"));
	layer_add_child(layer_last_checkin, text_layer_get_layer(text_layer_last_checkin_venue));

	// Date
	#ifdef PBL_ROUND
		text_layer_last_checkin_date = text_layer_create(GRect(0,(bounds.size.h/2)+10,SCREEN_WIDTH,20));
	#else
	 	text_layer_last_checkin_date = text_layer_create(GRect(10,124,bounds.size.w,20));
	#endif
	text_layer_set_text_color(text_layer_last_checkin_date, GColorBlack);
	text_layer_set_background_color(text_layer_last_checkin_date, GColorClear);
	text_layer_set_text_alignment(text_layer_last_checkin_date, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
	text_layer_set_font(text_layer_last_checkin_date, fonts_get_system_font(FONT_KEY_GOTHIC_14));

	text_layer_set_text(text_layer_last_checkin_date, _("at sometime..."));
	layer_add_child(layer_last_checkin, text_layer_get_layer(text_layer_last_checkin_date));

	// TODO: Not sure about address placement here...
	//layer_add_child(layer_last_checkin, text_layer_get_layer(text_layer_last_checkin_address));

	layer_add_child(window_layer, layer_last_checkin);

	// Menu
	layer_menu_venues = menu_layer_create(
		GRect(bounds.origin.x, bounds.origin.y+bounds.size.h, bounds.size.w, bounds.size.h));
	menu_layer_set_callbacks(layer_menu_venues, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.draw_header = menu_draw_header_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_row = menu_draw_row_callback
	});
	menu_mode = false;

	image_cog = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COG);

	#ifdef PBL_SDK_3
		#ifdef PBL_COLOR
			menu_layer_set_normal_colors(layer_menu_venues, GColorWhite, GColorDarkGray);
			if(get_primary_color() == GColorYellow.argb)
				menu_layer_set_highlight_colors(layer_menu_venues, (GColor)get_primary_color(), GColorBlack);
			else
				menu_layer_set_highlight_colors(layer_menu_venues, (GColor)get_primary_color(), GColorWhite);
		#else
			menu_layer_set_normal_colors(layer_menu_venues, GColorWhite, GColorBlack);
			menu_layer_set_highlight_colors(layer_menu_venues, GColorBlack, GColorWhite);
		#endif
	#endif
	layer_add_child(window_layer, menu_layer_get_layer(layer_menu_venues));

	// Text Background 1
	layer_primary_back = layer_create(GRect(0,(bounds.size.h/2) - STATUS_BAR_OFFSET,bounds.size.w, (bounds.size.h/2) + STATUS_BAR_OFFSET));
	layer_set_update_proc(layer_primary_back, draw_layer_primary_back);
	layer_add_child(window_layer, layer_primary_back);

	// Text 1
	#ifdef PBL_ROUND
		text_layer_primary = text_layer_create(GRect(15,5,bounds.size.w-25,74));
   		text_layer_enable_screen_text_flow_and_paging(text_layer_primary, 5);
	#else
		text_layer_primary = text_layer_create(GRect(10,10,bounds.size.w-10,74));
	#endif
	text_layer_set_text_color(text_layer_primary, GColorBlack);
	text_layer_set_background_color(text_layer_primary, GColorClear);
	text_layer_set_text_alignment(text_layer_primary, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
	if (strncmp(locale_current, "ru", 2) == 0) {
		text_layer_set_font(text_layer_primary, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	} else {
		text_layer_set_font(text_layer_primary, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	}
	text_layer_set_overflow_mode(text_layer_primary, GTextOverflowModeTrailingEllipsis);
	text_layer_set_text(text_layer_primary, _("Loading..."));
	layer_add_child(layer_primary_back, text_layer_get_layer(text_layer_primary));

	// Text 2
	#ifdef PBL_ROUND
		text_layer_primary_address = text_layer_create(GRect(30,50,bounds.size.w-60,20));
		text_layer_enable_screen_text_flow_and_paging(text_layer_primary_address, 10);
	#else
		text_layer_primary_address = text_layer_create(GRect(10,60,bounds.size.w-10,20));
	#endif
	text_layer_set_text_color(text_layer_primary_address, GColorBlack);
	text_layer_set_background_color(text_layer_primary_address, GColorClear);
	text_layer_set_text_alignment(text_layer_primary_address, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
	text_layer_set_font(text_layer_primary_address, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text(text_layer_primary_address, "");
	layer_add_child(layer_primary_back, text_layer_get_layer(text_layer_primary_address));
	
	#ifdef PBL_ROUND
		text_layer_primary_distance = text_layer_create(GRect(0,65,bounds.size.w,20));
		text_layer_enable_screen_text_flow_and_paging(text_layer_primary_distance, 5);
		text_layer_set_background_color(text_layer_primary_distance, GColorClear);
		text_layer_set_text_alignment(text_layer_primary_distance, GTextAlignmentCenter);
		text_layer_set_font(text_layer_primary_distance, fonts_get_system_font(FONT_KEY_GOTHIC_14));
		text_layer_set_text(text_layer_primary_distance, "");
		layer_add_child(layer_primary_back, text_layer_get_layer(text_layer_primary_distance));
	#endif

	// FAB
	layer_primary_circle = layer_create(GRect(0,PBL_IF_ROUND_ELSE(0,0 - STATUS_BAR_OFFSET),bounds.size.w,bounds.size.h));
 	layer_set_update_proc(layer_primary_circle, draw_layer_primary_circle);
	layer_add_child(window_layer, layer_primary_circle);

	image_refresh = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REFRESH);

	// Status Bar
	#ifdef PBL_SDK_3
		s_status_bar = status_bar_layer_create();
		status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
		#ifdef PBL_COLOR
			status_bar_layer_set_colors(s_status_bar, PBL_IF_ROUND_ELSE((GColor)get_primary_color(), GColorClear), GColorBlack);
		#else
			status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
		#endif
		layer_add_child(
			window_layer, status_bar_layer_get_layer(s_status_bar));
	#endif

	reverse_menu_animation = false;

	if(bluetooth_connection_service_peek()) {
		if(persist_exists(KEY_TOKEN)) {
			no_foursquare = false;
		} else {
			no_foursquare = true;
 			text_layer_set_text(text_layer_primary, _("Connect to Foursquare on Phone"));
// 			text_layer_set_text(text_layer_primary, "Mos Eisley Cantina");
// 			text_layer_set_text(text_layer_primary_address, "24m - 7 Jawa Way");
		}
	} else {
		no_internet = true;
		text_layer_set_text(text_layer_primary, _("Error:\n No connection to phone"));
	}
}

static void window_unload(Window *window) {
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

void out_sent_handler(DictionaryIterator *sent, void *context) {
// 	APP_LOG(APP_LOG_LEVEL_DEBUG, "Out Sent");
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Out dropped: %i - %s", reason, translate_error(reason));
}

void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *text_tuple_token = dict_find(iter, SPOON_TOKEN);
	Tuple *text_tuple_latlng = dict_find(iter, SPOON_LOCATION);
	Tuple *text_tuple_result = dict_find(iter, SPOON_RESULT);
	Tuple *text_tuple_name = dict_find(iter, SPOON_NAME);
	Tuple *text_tuple_id = dict_find(iter, SPOON_ID);
	Tuple *text_tuple_address = dict_find(iter, SPOON_ADDRESS);
	Tuple *text_tuple_distance = dict_find(iter, SPOON_DISTANCE);
	Tuple *text_tuple_distance_unit = dict_find(iter, SPOON_UNIT);
	Tuple *text_tuple_error = dict_find(iter, SPOON_ERROR);
	Tuple *text_tuple_ready = dict_find(iter, SPOON_READY);
	Tuple *text_tuple_config = dict_find(iter, SPOON_CONFIG);

// 	APP_LOG(APP_LOG_LEVEL_DEBUG, "In received");

	if(text_tuple_error) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Error received from js");
		text_layer_set_text(text_layer_primary, getErrorReason(text_tuple_error->value->int16));
	} else if(text_tuple_config && text_tuple_distance_unit) {
		#ifdef PBL_COLOR
			int config = text_tuple_config->value->int16;
			config_init(config);
			persist_write_int(KEY_THEME, config);
			setup_theme_colors(config);
		#endif
		
		int unit = text_tuple_distance_unit->value->int16;
		persist_write_int(KEY_UNIT, unit);
		config_set_unit(unit);	
	} else if(text_tuple_ready) {
		if(!no_foursquare) {
			getListOfLocations();
		}
	} else if(text_tuple_token && !text_tuple_latlng) {
		text_layer_set_text(text_layer_primary, _("Connected to Foursquare!"));

		persist_write_string(KEY_TOKEN, text_tuple_token->value->cstring);
		persist_exists(KEY_TOKEN);
		char key_stored[50];
		persist_read_string(KEY_TOKEN, key_stored, sizeof(key_stored));
		no_foursquare = false;
	} else if(text_tuple_result) {
		checkin_result_receiver((bool)text_tuple_result->value->int16);
	} else if(!text_tuple_token) {
		int index = dict_find(iter, SPOON_INDEX)->value->int16;
		if(text_tuple_name) {
			SpoonVenue venue;
			venue.index = index;
			strncpy(venue.id, text_tuple_id->value->cstring, sizeof(venue.id));
			strcpy(venue.name, text_tuple_name->value->cstring);

			if(text_tuple_address && strlen(text_tuple_address->value->cstring) > 0) {
				strcpy(venue.address, text_tuple_address->value->cstring);
			} else {
				strcpy(venue.address, _("No Address"));
			}
			
			if(text_tuple_distance && text_tuple_distance_unit) {
				strcpy(venue.distance, text_tuple_distance->value->cstring);
				venue.distance_unit = text_tuple_distance_unit->value->int16;
			}

			if(index == -1) {
				venue.isRecent = true;
				lastCheckinVenue = venue;

				text_layer_set_text(text_layer_last_checkin_venue, lastCheckinVenue.name);
				// HACK: Using the address field for date here
 				text_layer_set_text(text_layer_last_checkin_date, lastCheckinVenue.address);
			} else {
				venue.isRecent = false;
				venues[venue.index] = venue;
				num_venues++;
			}

			if(venue.index == 0) {
				text_layer_set_size(text_layer_primary, GSize(SCREEN_WIDTH-40,50));
				text_layer_set_text(text_layer_primary, venues[0].name);
				static char addressString[75];
				#ifdef PBL_ROUND
					text_layer_set_text(text_layer_primary_address, venues[0].address);
					snprintf(addressString, sizeof(addressString), 
							 "%s %s", venues[0].distance, get_unit(venues[0].distance_unit));
					text_layer_set_text(text_layer_primary_distance, addressString);
				#else
					snprintf(addressString, sizeof(addressString), 
							 "%s %s - %s", venues[0].distance, get_unit(venues[0].distance_unit), venues[0].address);
					text_layer_set_text(text_layer_primary_address, addressString);
				#endif
				layer_mark_dirty(text_layer_get_layer(text_layer_primary));
				layer_mark_dirty(layer_primary_back);
			}
			menu_layer_reload_data_and_mark_dirty(layer_menu_venues);

			if(index == MAX_VENUES - 1) {
				is_refreshing = false;
				vibes_short_pulse();
			}
		}
	} else {
		if(!text_tuple_token) {
			text_layer_set_text(text_layer_primary, _("Connect to Foursquare on Phone"));
		} else {
			text_layer_set_text(text_layer_primary, _("Cannot determine current location. :("));
		}
	}
}

void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Hello. It's me, I dropped a message: %i - %s", reason, translate_error(reason));
}

static void update_app_glance(AppGlanceReloadSession *session,
							  size_t limit, void *context) {
	if(limit < 1) return;
	
	char *message = "";
	uint32_t icon = RESOURCE_ID_IMAGE_SPOON_WHITE;
	time_t temp_time = time(NULL);
	time_t expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION;
	
	// TODO: Translate new string
// 	strncpy(message, _("Go explore!"), 20);
	strncpy(message, "Go explore!", 20);

	if(!no_foursquare) {
		if(strlen(lastCheckinVenue.name) > 0) {
			strncpy(message, lastCheckinVenue.name, 25);
			icon = RESOURCE_ID_IMAGE_CHECK_ON;
			expiration_time = temp_time + DAY_IN_SECONDS;
		} else {
			// TODO: Give them a random friendly message
		}
	} else {
		strncpy(message, _("Connect to Foursquare on Phone"), 40);
	}
	
	const char *glance_message = message;
	const uint32_t glance_icon = icon;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "message (before sending): %s", message);
	const AppGlanceSlice slice = (AppGlanceSlice) {
		.layout = {
			.subtitle_template_string = glance_message,
			.icon = glance_icon
		},
		.expiration_time = expiration_time
	};

	const AppGlanceResult result = app_glance_add_slice(session, slice);
}

static void init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
	#ifdef PBL_PLATFORM_APLITE
		app_message_open(128, 128);
	#else
		app_message_open(256, 128);
	#endif

	// Detect system locale
    locale_current = i18n_get_system_locale();

    if (strncmp(locale_current, "fr", 2) == 0) {
        localize_init(RESOURCE_ID_LOCALE_FRENCH);
    } else if (strncmp(locale_current, "es", 2) == 0) {
        localize_init(RESOURCE_ID_LOCALE_SPANISH);
    } else if (strncmp(locale_current, "de", 2) == 0) {
        localize_init(RESOURCE_ID_LOCALE_GERMAN);
    } else {
		localize_init(RESOURCE_ID_LOCALE_ENGLISH);
	}
	
	localize_set_cache_size(2);
	
	s_main_window = window_create();
	window_set_click_config_provider(s_main_window, click_config_provider);
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(s_main_window, true);
}

static void deinit(void) {
  	animation_unschedule_all();
	
	app_glance_reload(update_app_glance, NULL);

	text_layer_destroy_safe(text_layer_last_checkin_title);
	text_layer_destroy_safe(text_layer_last_checkin_venue);
	text_layer_destroy_safe(text_layer_last_checkin_address);
	text_layer_destroy_safe(text_layer_primary_distance);
	text_layer_destroy_safe(text_layer_last_checkin_date);

	layer_destroy_safe(layer_last_checkin);
	text_layer_destroy_safe(text_layer_primary);
	text_layer_destroy_safe(text_layer_primary_address);
	layer_destroy_safe(layer_primary_back);
	layer_destroy_safe(layer_back);

	bitmap_layer_destroy_safe(image_layer_back);
	gbitmap_destroy(image_cog);
	gbitmap_destroy(image_refresh);

	window_destroy_safe(s_main_window);
	checkin_deinit();
	checkin_menu_deinit();
	
	localize_deinit();

	#ifdef PBL_SDK_2
		property_animation_destroy(s_drop_current_animation);
		property_animation_destroy(s_drop_last_animation);
		property_animation_destroy(s_transition_menu_animation);
		property_animation_destroy(s_transition_circle_animation);
		property_animation_destroy(s_transition_box_animation);
		property_animation_destroy(s_transition_text_1_animation);
		property_animation_destroy(s_transition_text_2_animation);
	#endif
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}