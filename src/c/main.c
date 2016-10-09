#include <pebble.h>
#include "settings.h"
 
Window *s_main_window;
Layer *main_layer;
TextLayer *s_date_layer, *s_time_layer, *s_alert_layer;
bool bt_connected = false;
struct tm now;

int sel_font = 0;
uint32_t color_bg = 0x000000;
uint32_t color_date = 0xffffff;
uint32_t color_time = 0xffffff;

#define TIME_H PBL_IF_RECT_ELSE(48, 64)
#define DATE_H 28
#define ICON_H 52
  

static void bt_handler(bool connected) {
  bt_connected = connected;
  if (! bt_connected) {
    vibes_double_pulse();
  }
  layer_mark_dirty(main_layer);
}

static void window_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorFromHEX(color_bg));
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

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
  strftime(s_date_text, sizeof(s_date_text), "%d %b", &now);
  text_layer_set_text(s_date_layer, NOZERO(s_date_text));

  // Time
  if (clock_is_24h_style()) {
    strftime(s_time_text, sizeof(s_time_text), "%R", &now);
  } else {
    strftime(s_time_text, sizeof(s_time_text), "%I:%M", &now);
  }
  text_layer_set_text(s_time_layer, NOZERO(s_time_text));
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  now = *tick_time;
  layer_mark_dirty(main_layer);
}

static void reposition(GRect bounds) {
  int16_t bottom = bounds.origin.y + bounds.size.h;
  int16_t margin_b = TIME_H + bounds.size.h/6;
  int16_t margin_l = 7;
  GRect r;
  Layer *l;
  
  r.origin.x = bounds.origin.x + margin_l;
  r.origin.y = PBL_IF_RECT_ELSE(bottom - margin_b, (bottom - TIME_H)/2 - 10);
  r.size.w = bounds.size.w - margin_l * 2;
  r.size.h = TIME_H * 5 / 4;
  
  // Time
  l = text_layer_get_layer(s_time_layer);
  layer_set_frame(l, r);
  layer_mark_dirty(l);
  
  // Date
  l = text_layer_get_layer(s_date_layer);
  r.origin.y -= 26;
  r.origin.x += 1;
  r.size.w -= 2;
  layer_set_frame(l, r);
  layer_mark_dirty(l);
  
  // Alert
  l = text_layer_get_layer(s_alert_layer);
  r.origin.y = PBL_IF_RECT_ELSE(margin_l, bottom - ICON_H - margin_b);
  layer_set_frame(l, r);
  layer_mark_dirty(l);
}

static void main_window_load(Window *window) {
  GRect main_frame;
  
  main_layer = window_get_root_layer(window);
  main_frame = layer_get_frame(main_layer);

  s_date_layer = text_layer_create(main_frame);
  s_time_layer = text_layer_create(main_frame);
  s_alert_layer = text_layer_create(main_frame);
  text_layer_set_text_alignment(s_date_layer, PBL_IF_RECT_ELSE(GTextAlignmentLeft, GTextAlignmentCenter));
  text_layer_set_text_alignment(s_time_layer, PBL_IF_RECT_ELSE(GTextAlignmentLeft, GTextAlignmentCenter));
  text_layer_set_text_alignment(s_alert_layer, PBL_IF_RECT_ELSE(GTextAlignmentRight, GTextAlignmentCenter));
  reposition(main_frame);

  text_layer_set_background_color(s_date_layer, GColorClear);
  layer_add_child(main_layer, text_layer_get_layer(s_date_layer));

  text_layer_set_background_color(s_time_layer, GColorClear);
  layer_add_child(main_layer, text_layer_get_layer(s_time_layer));
  
  text_layer_set_background_color(s_alert_layer, GColorClear);
  text_layer_set_font(s_alert_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SYMBOLS_52)));
  layer_add_child(main_layer, text_layer_get_layer(s_alert_layer));
  
  layer_set_update_proc(main_layer, window_update_proc);
}

static void restore() {
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
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%x %06lx %06lx %06lx", sel_font, color_bg, color_date, color_time);
  
  text_layer_set_text_color(s_date_layer, GColorFromHEX(color_date));
  text_layer_set_text_color(s_time_layer, GColorFromHEX(color_time));
  text_layer_set_text_color(s_alert_layer, GColorFromHEX(color_time));
  
  text_layer_set_font(s_date_layer, fonts_load_custom_font(resource_get_handle(fonts[sel_font][0])));
  text_layer_set_font(s_time_layer, fonts_load_custom_font(resource_get_handle(fonts[sel_font][1])));
  
  layer_mark_dirty(main_layer);
}

static void unobstructedAreaWillChange(GRect final_unobstructed_screen_area, void *context) {
  reposition(final_unobstructed_screen_area);
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
  UnobstructedAreaHandlers uahandlers = {
    .will_change = unobstructedAreaWillChange
  };

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
  
  unobstructed_area_service_subscribe(uahandlers, NULL);
  
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