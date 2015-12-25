#include <pebble.h>
#include "settings.h"
 
static Window *s_main_window;
static TextLayer *s_date_layer, *s_time_layer, *s_alert_layer;
bool bt_connected = false;

static void bt_handler(bool connected) {
  bt_connected = connected;
  if (! bt_connected) {
    vibes_double_pulse();
  }
  layer_mark_dirty(text_layer_get_layer(s_alert_layer));
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char s_time_text[] = "00:00";
  static char s_date_text[] = "00 Çêû";

  // Bluetooth alert
  if (bt_connected) {
    text_layer_set_text(s_alert_layer, "");
  } else {
    text_layer_set_text(s_alert_layer, "");
  }
  
  // Date
  strftime(s_date_text, sizeof(s_date_text), "%d %b", tick_time);
  text_layer_set_text(s_date_layer, NOZERO(s_date_text));

  // Time
  if (clock_is_24h_style()) {
    strftime(s_time_text, sizeof(s_time_text), "%R", tick_time);
  } else {
    strftime(s_time_text, sizeof(s_time_text), "%I:%M", tick_time);
  }
  text_layer_set_text(s_time_layer, NOZERO(s_time_text));
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

#ifdef PBL_RECT
  s_date_layer = text_layer_create(GRect(8, 66, 128, 100));
  s_time_layer = text_layer_create(GRect(7, 92, 130, 76));
  s_alert_layer = text_layer_create(GRect(110, 7, 52, 52));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);  
#else
  s_date_layer = text_layer_create(GRect(0, 20, 180, 100));
  s_time_layer = text_layer_create(GRect(0, 48, 180, 76));
  s_alert_layer = text_layer_create(GRect(0, 120, 180, 52));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);  
  text_layer_set_text_alignment(s_alert_layer, GTextAlignmentCenter);  
#endif

  text_layer_set_background_color(s_date_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  text_layer_set_background_color(s_time_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  text_layer_set_background_color(s_alert_layer, GColorClear);
  text_layer_set_font(s_alert_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SYMBOLS_52)));
  layer_add_child(window_layer, text_layer_get_layer(s_alert_layer));
}

static void restore() {
  int sel_font = 0;
  uint32_t color_bg = 0x000000;
  uint32_t color_date = 0xffffff;
  uint32_t color_time = 0xffffff;
  
  int i;
  for (i = 0; i < KEY_LAST; i += 1) {
    if (! persist_exists(i)) {
      continue;
    }
    
    switch (i) {
    case KEY_FONT:
      sel_font = persist_read_int(i);
      break;
    case KEY_COLOR_BG:
      color_bg = persist_read_int(i);
      break;
    case KEY_COLOR_DATE:
      color_date = persist_read_int(i);
      break;
    case KEY_COLOR_TIME:
      color_time = persist_read_int(i);
      break;      
    }
  }
  
  window_set_background_color(s_main_window, GColorFromHEX(color_bg));
  text_layer_set_text_color(s_date_layer, GColorFromHEX(color_date));
  text_layer_set_text_color(s_time_layer, GColorFromHEX(color_time));
  text_layer_set_text_color(s_alert_layer, GColorFromHEX(color_time));
  
  text_layer_set_font(s_date_layer, fonts_load_custom_font(resource_get_handle(fonts[sel_font][0])));
  text_layer_set_font(s_time_layer, fonts_load_custom_font(resource_get_handle(fonts[sel_font][1])));
}

static void in_received_handler(DictionaryIterator *rec, void *context) {
  int i;
  
  for (i = 0; i < KEY_LAST; i += 1) {
    Tuple *cur = dict_find(rec, i);
    
    if (! cur) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Holy crap! Key %i isn't around!", i);
      continue;
    }

    // They're all ints! Yay!
    persist_write_int(i, cur->value->int32);
  }
  
  restore();
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  // XXX: I don't understand what we could possibly do here
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_alert_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  bluetooth_connection_service_subscribe(bt_handler);
  bt_connected = bluetooth_connection_service_peek();
  
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_open(256, 64);
  
  // Prevent starting blank
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  handle_minute_tick(t, MINUTE_UNIT);
  
  setlocale(LC_ALL, "");
  
  restore();
}

static void deinit() {
  window_destroy(s_main_window);

  tick_timer_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}