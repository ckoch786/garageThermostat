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
#include "display.h"
#include "garageThermostat.h"


struct display_control display_cntrl = {}; 



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

// ========== Main Program ==========
int main(void) {
  char buffer[16];
  SHT41_Data sensor_data;

  // Initialize peripherals
  uart_init(9600);
  i2c_init();
  //adc_init();
  analog_comp_init();
  _delay_ms(100);

  uart_puts("Garage Control Center\r\n");

  hcsr04_init();
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

  init_timer1();

  // Enable global interrupts
  sei();

  uart_puts("System initialized. Waiting for light trigger...\r\n");

  // Main loop
  while (1) {
    // Only update display when flag is set by interrupt
    // uint8_t displayOn = (needs_update && display_on);
    uint8_t displayOn = 0;
    if (display_cntrl.broadcast || displayOn) {
      //if(1) {
      display_cntrl.needs_update = 0;  // Clear flag

      if (!displayOn) {
        ssd1306_clear();
      }

      // Read sensor
      if (sht41_read_measurement(&sensor_data)) {
        uint8_t remaining = DISPLAY_TIMEOUT_SEC - display_cntrl.timer_seconds;
        if (!display_cntrl.broadcast) {
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
          ssd1306_puts("TIME:", 0, 7, 1);
          if (remaining < 10) {
            ssd1306_putchar('0' + remaining, 1);
          } else {
            ssd1306_putchar('0' + (remaining / 10), 1);
            ssd1306_putchar('0' + (remaining % 10), 1);
          }

          uint32_t distance_cm = hcsr04_read();

          if (distance_cm <= 25) {
            //ssd1306_clear();
            ssd1306_puts("ON ", 100, 6, 4);
          } else {
            ssd1306_puts("OFF", 100, 6, 4);
          }

          ssd1306_puts("DISTANCE:", 100, 2, 1);
          sprintf(buffer, "%lu cm", distance_cm);
          ssd1306_puts(buffer, 100, 3, 1);
        }

        // TODO change this to buffer and only print every so often
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
