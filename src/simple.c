#include "pebble.h"

static Window *s_main_window;
static TextLayer *s_date1_layer, *s_date2_layer, *s_time_layer;
static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;
/*///////////////////////////////////////////////////////////
//
// CHANGE "hourlyVibeIsEnabled" TO TRUE TO ADD HOURLY VIBES
// CHANGE "useOldFont" TO TRUE TO USE THE OLD MORE BOLD FONT
//
*////////////////////////////////////////////////////////////
bool hourlyVibeIsEnabled = false;
bool useOldFont = false;
bool init_wf = false;

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  // Issue a vibrating alert
  if ((init_wf && !connected) || !init_wf)
    vibes_double_pulse();
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) 
{	
  // Need to be static because they're used by the system later.
  static char s_date1_text[] = "Xxxxxxx 00";
	static char s_time_text[] = "00:00";
	static char s_date2_text[] = "Xxx 0000";
	// Check to see if the font is small enought to handle a full weekday
	if (useOldFont)
	{
		strftime(s_date1_text, sizeof(s_date1_text), "%a %e", tick_time);
	}
	else
	{
		strftime(s_date1_text, sizeof(s_date1_text), "%a %e", tick_time);
	}
	strftime(s_date2_text, sizeof(s_date2_text), "%b %Y", tick_time);
	
	if (tick_time->tm_min == 0)
	{
		if (hourlyVibeIsEnabled)
		{
			if (tick_time->tm_hour >= 6 && tick_time->tm_hour <= 22)
			vibes_double_pulse();
		}
	}
	
  text_layer_set_text(s_date1_layer, s_date1_text);
	text_layer_set_text(s_date2_layer, s_date2_text);

  char *time_format;
  if (clock_is_24h_style()) 
	{
    time_format = "%R";
  } else 
	{
    time_format = "%I:%M";
  }
	
  strftime(s_time_text, sizeof(s_time_text), time_format, tick_time);

  // Handle lack of non-padded hour format string for twelve hour clock.
  if (!clock_is_24h_style() && (s_time_text[0] == '0')) 
	{
    memmove(s_time_text, &s_time_text[1], sizeof(s_time_text) - 1);
  }
  text_layer_set_text(s_time_layer, s_time_text);
}

static void main_window_load(Window *window) 
{
  Layer *window_layer = window_get_root_layer(window);
	
	//Check to see if the user wants the old font
	if (useOldFont)
	{
	s_date1_layer = text_layer_create(GRect(0, 37, 144, 30));
  text_layer_set_text_color(s_date1_layer, GColorWhite);
  text_layer_set_background_color(s_date1_layer, GColorClear);
	text_layer_set_font(s_date1_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARVO_22)));
	text_layer_set_text_alignment(s_date1_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date1_layer));
		
	s_time_layer = text_layer_create(GRect(0, 55, 144, 60));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_font(s_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARVO_BOLD_45)));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	
	s_date2_layer = text_layer_create(GRect(0, 100, 144, 30));
  text_layer_set_text_color(s_date2_layer, GColorWhite);
  text_layer_set_background_color(s_date2_layer, GColorClear);
	text_layer_set_font(s_date2_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARVO_22)));
	text_layer_set_text_alignment(s_date2_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date2_layer));
	}
	else
	{
	s_date1_layer = text_layer_create(GRect(0, 43, 144, 23));
  text_layer_set_text_color(s_date1_layer, GColorWhite);
  text_layer_set_background_color(s_date1_layer, GColorClear);
	text_layer_set_font(s_date1_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SAN_FRANCISCO_THIN_18)));
	text_layer_set_text_alignment(s_date1_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date1_layer));
		
	s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_font(s_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SAN_FRANCISCO_40)));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	
	s_date2_layer = text_layer_create(GRect(0, 95, 144, 25));
  text_layer_set_text_color(s_date2_layer, GColorWhite);
  text_layer_set_background_color(s_date2_layer, GColorClear);
	text_layer_set_font(s_date2_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SAN_FRANCISCO_THIN_18)));
	text_layer_set_text_alignment(s_date2_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date2_layer));
	}

  // Create the Bluetooth icon GBitmap
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);

  // Create the BitmapLayer to display the GBitmap
  s_bt_icon_layer = bitmap_layer_create(GRect(59, 12, 30, 30));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));

  // Show the correct state of the BT connection from the start
  init_wf = true;
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  init_wf = false;
}

static void main_window_unload(Window *window) 
{
  text_layer_destroy(s_date1_layer);
	text_layer_destroy(s_date2_layer);
  text_layer_destroy(s_time_layer);
  gbitmap_destroy(s_bt_icon_bitmap);
  bitmap_layer_destroy(s_bt_icon_layer);
}

static void init() 
{
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) 
	{
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  
  // Prevent starting blank
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  handle_minute_tick(t, MINUTE_UNIT);

  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
}

static void deinit()
{
  window_destroy(s_main_window);

  tick_timer_service_unsubscribe();
}

int main() 
{
  init();
  app_event_loop();
  deinit();
}
