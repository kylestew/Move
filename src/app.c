#include "pebble.h"
  
static GBitmap* image;
static BitmapLayer* background_layer;
TextLayer* text_layer;
//char buffer[] = "00:00";
static AppTimer* timer;
  

// =========================================
// == PEDOMETER ============================
// =========================================
// interval to check for next step (in ms)
const int ACCEL_STEP_MS = 475;
// value to auto adjust step acceptance 
const int PED_ADJUST = 2;

int X_DELTA = 35;
int Y_DELTA, Z_DELTA = 185;
int YZ_DELTA_MIN = 175;
int YZ_DELTA_MAX = 195; 
int X_DELTA_TEMP, Y_DELTA_TEMP, Z_DELTA_TEMP = 0;
int lastX, lastY, lastZ, currX, currY, currZ = 0;
int sensitivity = 1;

long pedometerCount = 0;
bool startedSession = false;
bool did_pebble_vibrate = false;
bool validX, validY, validZ = false;

void autoCorrectZ(){
	if (Z_DELTA > YZ_DELTA_MAX){
		Z_DELTA = YZ_DELTA_MAX; 
	} else if (Z_DELTA < YZ_DELTA_MIN){
		Z_DELTA = YZ_DELTA_MIN;
	}
}

void autoCorrectY(){
	if (Y_DELTA > YZ_DELTA_MAX){
		Y_DELTA = YZ_DELTA_MAX; 
	} else if (Y_DELTA < YZ_DELTA_MIN){
		Y_DELTA = YZ_DELTA_MIN;
	}
}

void pedometer_update() {
	if (startedSession) {
		X_DELTA_TEMP = abs(abs(currX) - abs(lastX));
		if (X_DELTA_TEMP >= X_DELTA) {
			validX = true;
		}
		Y_DELTA_TEMP = abs(abs(currY) - abs(lastY));
		if (Y_DELTA_TEMP >= Y_DELTA) {
			validY = true;
			if (Y_DELTA_TEMP - Y_DELTA > 200){
				autoCorrectY();
				Y_DELTA = (Y_DELTA < YZ_DELTA_MAX) ? Y_DELTA + PED_ADJUST : Y_DELTA;
			} else if (Y_DELTA - Y_DELTA_TEMP > 175){
				autoCorrectY();
				Y_DELTA = (Y_DELTA > YZ_DELTA_MIN) ? Y_DELTA - PED_ADJUST : Y_DELTA;
			}
		}
		Z_DELTA_TEMP = abs(abs(currZ) - abs(lastZ));
		if (abs(abs(currZ) - abs(lastZ)) >= Z_DELTA) {
			validZ = true;
			if (Z_DELTA_TEMP - Z_DELTA > 200){
				autoCorrectZ();
				Z_DELTA = (Z_DELTA < YZ_DELTA_MAX) ? Z_DELTA + PED_ADJUST : Z_DELTA;
			} else if (Z_DELTA - Z_DELTA_TEMP > 175){
				autoCorrectZ();
				Z_DELTA = (Z_DELTA < YZ_DELTA_MAX) ? Z_DELTA + PED_ADJUST : Z_DELTA;
			}
		}
	} else {
		startedSession = true;
	}  
}

void resetUpdate() {
	lastX = currX;
	lastY = currY;
	lastZ = currZ;
	validX = false;
	validY = false;
	validZ = false;
}

void update_activity_ui() {
  if ((validX && validY && !did_pebble_vibrate) || (validX && validZ && !did_pebble_vibrate)) {
		pedometerCount++;
    
    APP_LOG(APP_LOG_LEVEL_INFO, "STEPS: %ld", pedometerCount);
  }
  resetUpdate();
}
  
static void check_activity_timer_callback(void* data) {
  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
  accel_service_peek(&accel);
  
  if (!startedSession) {
		lastX = accel.x;
		lastY = accel.y;
		lastZ = accel.z;
	} else {
		currX = accel.x;
		currY = accel.y;
		currZ = accel.z;
	}
  
  // discard data if watch was vibrating
  did_pebble_vibrate = accel.did_vibrate;
  
  pedometer_update();
  update_activity_ui();
  
  timer = app_timer_register(ACCEL_STEP_MS , check_activity_timer_callback, NULL);  
}
// =========================================





void timer_callback(void* data) {
  // timer fired, annoy the user until they start moving
  vibes_long_pulse();
  
  // TODO: switch UI mode

  // == START PEDOMETER ==
  // start looking for continous motion
  accel_data_service_subscribe(0, NULL);
  timer = app_timer_register(ACCEL_STEP_MS, check_activity_timer_callback, NULL);  
}

// update clock UI
//void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
//  strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
//  text_layer_set_text(text_layer, buffer);  
//}

void setup() {
  // TEMP
  timer_callback(NULL);
  
  
//  // countdown timer
//  timer = app_timer_register(1000 * 30 * 1, timer_callback, NULL);
//
//  // track time for clock
//  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler)tick_handler);
}

void window_load(Window* window) {
  // background
//  image = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
// Layer* window_layer = window_get_root_layer(window);
//  GRect bounds = layer_get_frame(window_layer);
//  background_layer = bitmap_layer_create(bounds);
//  bitmap_layer_set_bitmap(background_layer, image);
//  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
  
//  // time text
//  text_layer = text_layer_create(GRect(0, 58, 143, 168 - 58));
//  text_layer_set_background_color(text_layer, GColorClear);
//  text_layer_set_text_color(text_layer, GColorWhite);
//  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
//  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));  
//  layer_add_child(window_get_root_layer(window), (Layer*)text_layer);
}

void window_unload(Window* window) {
//  tick_timer_service_unsubscribe();
//  text_layer_destroy(text_layer);
//  gbitmap_destroy(image);
//  bitmap_layer_destroy(background_layer);
}
