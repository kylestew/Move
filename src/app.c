#include <pebble.h>
#include "arc_draw.h"
#include "pedometer.h"
  
#define CIRCLE_RADIUS           68
#define CIRCLE_THICKNESS        14

#define MINUTES_BEFORE_BREAK    40
#define STEPS_UNTIL_RESET       120

static Window* window;
static Layer* pieLayer;
TextLayer* textLayer;
TextLayer* displayTextLayer;
static InverterLayer* invertLayer;
int minutesUntilBreak;
static AppTimer* timer;
bool activityMode = false;

static void handleTick(struct tm *tick_time, TimeUnits units_changed);


void switchToCountdownMode() {
  activityMode = false;
  vibes_long_pulse(); // signal user mode switch
  
  // shutdown accelerometer service
  accel_data_service_unsubscribe();
  
  // resets
  pedometerCount = 0;
  minutesUntilBreak = 0;

  inverter_layer_destroy(invertLayer);
  layer_mark_dirty(pieLayer);
  handleTick(NULL, MINUTE_UNIT); // update text
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
  vibes_long_pulse(); // signal user mode switch
  
  // invert UI
  GRect bounds = GRect(0, 24, 144, 144);
  invertLayer = inverter_layer_create(bounds);
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(invertLayer));
  
  // update display
  text_layer_set_text(displayTextLayer, "MOVE");  
  
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
    int sweep = (pedometerCount + 1) * (TRIG_MAX_ANGLE / STEPS_UNTIL_RESET);
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
  if (activityMode == true) {
    // MODE: active
    // annoy user every 1 minute until they finish their exercise
    vibes_double_pulse();
  } else {
    // countdown
    minutesUntilBreak++;
    
    static char displayBuffer[3];
    snprintf(displayBuffer, sizeof(displayBuffer), "%d", MINUTES_BEFORE_BREAK - minutesUntilBreak);
    text_layer_set_text(displayTextLayer, displayBuffer);  
    
    if (minutesUntilBreak >= MINUTES_BEFORE_BREAK)
      switchToMoveMode();
    
    layer_mark_dirty(pieLayer);    
  }
  
  // update clock
  static char timeBuffer[10];
  clock_copy_time_string(timeBuffer, sizeof(timeBuffer));
  text_layer_set_text(textLayer, timeBuffer);  
}

void setup() {
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler)handleTick);
}

void window_load(Window* w) {
  window = w;
  window_set_background_color(window, GColorBlack);

  // pie-arc layer
  GRect bounds = GRect(0, 24, 144, 144);
  pieLayer = layer_create(bounds);
  layer_set_update_proc(pieLayer, pieLayerRenderCallback);
  layer_add_child(window_get_root_layer(window), pieLayer);
  
  // center display text
  displayTextLayer = text_layer_create(GRect(0, 62, 145, 64));
  text_layer_set_background_color(displayTextLayer, GColorClear);
  text_layer_set_text_color(displayTextLayer, GColorWhite);
  text_layer_set_text_alignment(displayTextLayer, GTextAlignmentCenter);
  GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AMERICAN_CAPTAIN_54));
  text_layer_set_font(displayTextLayer, custom_font);
  layer_add_child(window_get_root_layer(window), (Layer*)displayTextLayer);
  
  // time text
  textLayer = text_layer_create(GRect(0, 0, 145, 24));
  text_layer_set_background_color(textLayer, GColorClear);
  text_layer_set_text_color(textLayer, GColorWhite);
  text_layer_set_text_alignment(textLayer, GTextAlignmentCenter);
  text_layer_set_font(textLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_get_root_layer(window), (Layer*)textLayer);
}

void window_unload(Window* window) {
  tick_timer_service_unsubscribe();
  layer_destroy(pieLayer);
  text_layer_destroy(textLayer);
  text_layer_destroy(displayTextLayer);
}
