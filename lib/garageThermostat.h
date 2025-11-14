#ifndef GARAGE_THEMOSTAT_H
#define GARAGE_THEMOSTAT_H

/**
* TODO main documentation 
*/





// Configuration
#define CELCIUS 0  // 0 for Fahrenheit, 1 for Celsius

// Light sensor on ADC0 (PC0)
#define LIGHT_SENSOR_PIN 0
#define LIGHT_THRESHOLD 307  // ~1.5V threshold for display trigger

// Timer configuration for 30 second timeout
#define TIMER_PRESCALER 1024
#define DISPLAY_TIMEOUT_SEC 30


#endif


