#include "app.h"
#include <pebble.h>

static Window* window;

void alloc() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true); // animated
  setup();
}

void dealloc() {
  
}

int main(void) { 
  alloc(); 
  app_event_loop(); 
  dealloc(); 
}
