#ifndef GARAGE_THEMOSTAT_H
#define GARAGE_THEMOSTAT_H

/**
* TODO main documentation 
*/



struct display_control {
  volatile uint8_t display_on = 0;
  volatile uint16_t timer_seconds = 0;
  volatile uint8_t needs_update = 0;  // Flag for main loop
  volatile uint8_t broadcast = 0;
  volatile uint16_t broadcast_time_seconds = 0;
};

// Configuration
#define CELCIUS 0  // 0 for Fahrenheit, 1 for Celsius

// Light sensor on ADC0 (PC0)
#define LIGHT_SENSOR_PIN 0
#define LIGHT_THRESHOLD 307  // ~1.5V threshold for display trigger

// Timer configuration for 30 second timeout
#define TIMER_PRESCALER 1024
#define DISPLAY_TIMEOUT_SEC 30


#endif


