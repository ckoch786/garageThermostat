#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

struct display_control {
  volatile uint8_t display_on;
  volatile uint16_t timer_seconds;
  volatile uint8_t needs_update;  // Flag for main loop
  volatile uint8_t broadcast;
  volatile uint16_t broadcast_time_seconds;
};

void enable_display(struct display_control *dc);
void disable_display(struct display_control *dc);

#endif