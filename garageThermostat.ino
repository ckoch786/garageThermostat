#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "adc.h"
#include "i2c.h"
#include "temperature_SHT41.h"
#include "OLED_SSD1306.h"


// Configuration
#define CELCIUS 0  // 0 for Fahrenheit, 1 for Celsius

// Light sensor on ADC0 (PC0)
#define LIGHT_SENSOR_PIN 0
#define LIGHT_THRESHOLD 307    // ~1.5V threshold for display trigger

// Timer configuration for 30 second timeout
#define TIMER_PRESCALER 1024
#define DISPLAY_TIMEOUT_SEC 30

// Global state variables
volatile uint8_t display_on = 0;
volatile uint16_t timer_seconds = 0;
volatile uint8_t needs_update = 0;  // Flag for main loop


// ========== Display Control Functions ==========
void enable_display(void) {
  display_on = 1;
  timer_seconds = 0;
  TCNT1 = 0;
  needs_update = 1;  // Signal main loop to update
}

void disable_display(void) {
  if (display_on) {
    display_on = 0;
    timer_seconds = 0;
    TCNT1 = 0;
    ssd1306_clear();  // Clear when turning off
  }
}

// ========== Timer Setup ==========
void init_timer1(void) {
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC mode, prescaler 1024
  OCR1A = (F_CPU / TIMER_PRESCALER) - 1;  // ~15624 for 1 second
  TIMSK1 = (1 << OCIE1A);  // Enable compare match interrupt
}

// ========== Pin Change Interrupt Setup ==========
void init_light_sensor_interrupt(void) {
  DDRC &= ~(1 << PC0);     // PC0 as input
  PCMSK1 |= (1 << PCINT8); // Enable PCINT8 (PC0)
  PCICR |= (1 << PCIE1);   // Enable PCINT1 interrupt group
}

// ========== ISRs ==========

// Pin Change Interrupt - triggers on light sensor activity
ISR(PCINT1_vect) {
  // Check if PC0 is HIGH (light detected)
  if (PINC & (1 << PC0)) {
    // Read ADC to confirm threshold
    uint16_t sensor_value = adc_read(LIGHT_SENSOR_PIN);
    if (sensor_value > LIGHT_THRESHOLD) {
      enable_display();
    }
  }
}

// Timer1 Compare Match - called every second
ISR(TIMER1_COMPA_vect) {
  if (display_on) {
    timer_seconds++;
    if (timer_seconds >= DISPLAY_TIMEOUT_SEC) {
      disable_display();
    } else {
      needs_update = 1;  // Update display every second
    }
  }
}

// ========== Main Program ==========
int main(void) {
  SHT41_Data sensor_data;

  // Initialize peripherals
  uart_init(9600);
  i2c_init();
  adc_init();
  _delay_ms(100);

  uart_puts("Garage Control Center\r\n");

  // Initialize OLED
  ssd1306_init();
  ssd1306_clear();

  // Display title
  ssd1306_puts("SHT41 SENSOR", 10, 0, 1);

  // Soft reset sensor
  if (sht41_soft_reset()) {
    ssd1306_puts("INIT OK", 30, 1, 1);
    uart_puts("Sensor reset OK\r\n");
  } else {
    ssd1306_puts("INIT FAIL", 25, 1, 1);
    uart_puts("Sensor reset failed\r\n");
  }

  _delay_ms(2000);
  ssd1306_clear();

  // Initialize interrupts
  // TODO need to use the comparator to handle this instead.
  init_light_sensor_interrupt();
  init_timer1();
  
  // Enable global interrupts
  sei();

  uart_puts("System initialized. Waiting for light trigger...\r\n");

  // Main loop
  while (1) {
    // Only update display when flag is set by interrupt
    //if (needs_update && display_on) {
    if(1) {
      needs_update = 0;  // Clear flag
      
      // Read sensor
      if (sht41_read_measurement(&sensor_data)) {
        // Clear and update display
        ssd1306_clear();

        // Display temperature
        ssd1306_puts("TEMP:", 0, 1, 1);
        ssd1306_print_float(sensor_data.temperature, 1, 0, 2, 1);
#if CELCIUS
        ssd1306_puts("C", 50, 2, 1);
#else
        ssd1306_puts("F", 50, 2, 1);
#endif

        // Display humidity
        ssd1306_puts("HUMID:", 0, 4, 1);
        ssd1306_print_float(sensor_data.humidity, 1, 0, 5, 1);
        ssd1306_puts("%", 50, 5, 1);

        // Display timeout countdown
        uint8_t remaining = DISPLAY_TIMEOUT_SEC - timer_seconds;
        ssd1306_puts("TIME:", 0, 7, 1);
        if (remaining < 10) {
          ssd1306_putchar('0' + remaining, 1);
        } else {
          ssd1306_putchar('0' + (remaining / 10), 1);
          ssd1306_putchar('0' + (remaining % 10), 1);
        }

        // UART output
        uart_puts("Temperature: ");
        uart_print_float(sensor_data.temperature, 1);
#if CELCIUS
        uart_puts(" C Humidity: ");
#else
        uart_puts(" F Humidity: ");
#endif
        uart_print_float(sensor_data.humidity, 1);
        uart_puts(" % Time: ");
        uart_print_uint16(remaining);
        uart_puts("s\r\n");
      } else {
        ssd1306_clear();
        ssd1306_puts("READ ERROR", 20, 3, 1);
        uart_puts("Measurement failed\r\n");
      }
    }
    
    // Low power idle - wait for interrupts
    _delay_ms(50);
  }

  return 0;
}
