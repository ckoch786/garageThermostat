#ifndef DISPLAY_H
#define DISPLAY_H

struct display_control {
  volatile uint8_t display_on = 0;
  volatile uint16_t timer_seconds = 0;
  volatile uint8_t needs_update = 0;  // Flag for main loop
  volatile uint8_t broadcast = 0;
  volatile uint16_t broadcast_time_seconds = 0;
};

void enable_display(struct display_control *dc);
void disable_display(struct display_control *dc);

#endif