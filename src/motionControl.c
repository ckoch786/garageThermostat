#include "motionControl.h"
#include "display.h"
#include "adc.h"
#include "garageThermostat.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Define the global display control instance
struct display_control display_cntrl = {0};


void motionControl_init(void) {
  motionControl_ddr |= (1 << motionControl_indicator_pin);  // Trig as output
}

void motionControl_toggleIndicator(void) {
  motionControl_indicator_port |= (1 << motionControl_indicator_pin);
  _delay_ms(1000); 
  motionControl_indicator_port &= ~(1 << motionControl_indicator_pin);
}


// ========== Timer Setup ==========
void init_timer1(void) {
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC mode, prescaler 1024
  OCR1A = (F_CPU / TIMER_PRESCALER) - 1;              // ~15624 for 1 second
  TIMSK1 = (1 << OCIE1A);                             // Enable compare match interrupt
}


// ========== ISRs ==========
// #define ANALOG_COMP_vect_num  23
// #define ANALOG_COMP_vect  _VECTOR(23)  /* Analog Comparator */
ISR(ANALOG_COMP_vect) {
  display_cntrl.display_on = 1;
}

void analog_comp_init(void) {
  // Set bandgap reference (1.1V) on negative input
  ACSR &= ~(1 << ACBG);

  // Enable analog comparator interrupt
  ACSR |= (1 << ACIE);

  // Set interrupt on rising edge (low to high transition)
  ACSR |= (1 << ACIS1) | (1 << ACIS0);

  sei();
}


// Pin Change Interrupt - triggers on light sensor activity
ISR(PCINT1_vect) {
  // Check if PC0 is HIGH (light detected)
  if (PINC & (1 << PC0)) {
    // Read ADC to confirm threshold
    uint16_t sensor_value = adc_read(LIGHT_SENSOR_PIN);
    if (sensor_value > LIGHT_THRESHOLD) {
      enable_display(&display_cntrl);
    }
  }
}

// Timer1 Compare Match - called every second
ISR(TIMER1_COMPA_vect) {
  if (display_cntrl.display_on) {
    display_cntrl.timer_seconds++;
    if (display_cntrl.timer_seconds >= DISPLAY_TIMEOUT_SEC) {
      disable_display(&display_cntrl);
    } else {
      display_cntrl.needs_update = 1;  // Update display every second
    }
  }

  // Broadcast timer
  display_cntrl.broadcast_time_seconds++;
  if (display_cntrl.broadcast_time_seconds % 10 == 0) {  // broadcast every 10 seconds
    display_cntrl.broadcast = 1;
  } else {
    display_cntrl.broadcast = 0;
  }
}
