#include <pebble.h>
#include "arc_draw.h"
#include "pedometer.h"
  
#define CIRCLE_RADIUS           68
#define CIRCLE_THICKNESS        14

#define MINUTES_BEFORE_BREAK    20
#define STEPS_UNTIL_RESET       20

static Window* window;
static Layer* pieLayer;
//TextLayer* text_layer;
static InverterLayer* invertLayer;
//char buffer[] = "00:00";
int32_t minutesUntilBreak;
static AppTimer* timer;
bool activityMode = false;


void switchToCountdownMode() {
  activityMode = false;

  // shutdown accelerometer service
  accel_data_service_unsubscribe();
  
  // resets
  pedometerCount = 0;
  minutesUntilBreak = MINUTES_BEFORE_BREAK;

  inverter_layer_destroy(invertLayer);
  layer_mark_dirty(pieLayer);
}

static void check_activity_timer_callback(void* data) {
  if (!activityMode)
    return;
  
  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
  accel_service_peek(&accel);
  
  pedometer_update(accel);
  layer_mark_dirty(pieLayer);
  
  if (pedometerCount > STEPS_UNTIL_RESET) {
    switchToCountdownMode();
  } else {
    timer = app_timer_register(ACCEL_STEP_MS , check_activity_timer_callback, NULL);      
  }
}

void switchToMoveMode() {
  if (activityMode == true)
    return;
  activityMode = true;
  
  // invert UI
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(invertLayer));
  
  // TODO: change text
  
  // == START PEDOMETER ==
  // start looking for continous motion
  accel_data_service_subscribe(0, NULL);
  timer = app_timer_register(ACCEL_STEP_MS, check_activity_timer_callback, NULL); 
}

static void pieLayerRenderCallback(Layer* me, GContext *ctx) {
  GPoint center = GPoint(72, 72);
  int start, end;
  if (activityMode) {
    start = angle_270;
    int sweep = pedometerCount * (TRIG_MAX_ANGLE / STEPS_UNTIL_RESET);
    sweep = sweep > TRIG_MAX_ANGLE ? TRIG_MAX_ANGLE : sweep; // max out angle swept
    sweep = sweep == 0 ? angle_270 + angle_90 : sweep;
    end = start - sweep;
  } else {
    start = -angle_90;
    int sweep = minutesUntilBreak * (TRIG_MAX_ANGLE / MINUTES_BEFORE_BREAK);
    sweep = sweep > TRIG_MAX_ANGLE ? TRIG_MAX_ANGLE : sweep; // max out angle swept
    end = sweep + start;
  }
  graphics_draw_arc(ctx, center, CIRCLE_RADIUS, CIRCLE_THICKNESS, start, end, GColorWhite);
}

static void handleTick(struct tm *tick_time, TimeUnits units_changed) {
  minutesUntilBreak++;
  if (minutesUntilBreak >= MINUTES_BEFORE_BREAK) {
    // TODO: annoy user every 1 minute until they finish their exercise
//    vibes_long_pulse();
    
    switchToMoveMode();
  } else {
    layer_mark_dirty(pieLayer);    
  }
  
  // TODO: update time display
//  strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
//  text_layer_set_text(text_layer, buffer);  
}

void setup() {
  // TODO: change to minutes
  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler)handleTick);
}

void window_load(Window* w) {
  window = w;
  window_set_background_color(window, GColorBlack);

  // pie-arc layer
  GRect bounds = GRect(0, 24, 144, 144);
  pieLayer = layer_create(bounds);
  layer_set_update_proc(pieLayer, pieLayerRenderCallback);
  layer_add_child(window_get_root_layer(window), pieLayer);
  
//  // time text
//  text_layer = text_layer_create(GRect(0, 58, 143, 168 - 58));
//  text_layer_set_background_color(text_layer, GColorClear);
//  text_layer_set_text_color(text_layer, GColorWhite);
//  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
//  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));  
//  layer_add_child(window_get_root_layer(window), (Layer*)text_layer);
  
  // inverter layer (prepare)
  invertLayer = inverter_layer_create(bounds);
}

void window_unload(Window* window) {
  tick_timer_service_unsubscribe();
  layer_destroy(pieLayer);
  inverter_layer_destroy(invertLayer);
}
