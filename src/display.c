#include "display.h"
#include "OLED_SSD1306.h"
#include <avr/io.h>
#include <util/delay.h>

// ========== Display Control Functions ==========
void enable_display(struct display_control *dc) {
  dc->display_on = 1;
  dc->timer_seconds = 0;
  TCNT1 = 0;
  dc->needs_update = 1;  // Signal main loop to update
}

void disable_display(struct display_control *dc) {
  if (dc->display_on) {
    dc->display_on = 0;
    dc->timer_seconds = 0;
    TCNT1 = 0;
    ssd1306_clear();  // Clear when turning off
  }
}