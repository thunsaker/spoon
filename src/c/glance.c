#include <pebble.h>
#include "src/c/glance.h"

char *glance_text;
uint32_t glance_icon;

static void prv_update_app_glance(AppGlanceReloadSession *session,
							  size_t limit, void *context) {
	if(limit < 1) return;

	time_t temp_time = time(NULL);
	time_t expiration_time = temp_time + HOUR_IN_SECONDS;

   const char *glance_message = context;
	const uint32_t final_glance_icon = glance_icon;

	const AppGlanceSlice slice = (AppGlanceSlice) {
		.layout = {
			.subtitle_template_string = glance_message,
			.icon = final_glance_icon
		},
		.expiration_time = expiration_time
	};

	const AppGlanceResult result = app_glance_add_slice(session, slice);
}

void update_app_glance(char *message, const uint32_t icon) {
   #if PBL_API_EXISTS(app_glance_reload)
      glance_text = message;
      glance_icon = icon;
      app_glance_reload(prv_update_app_glance, message);
   #endif
}
