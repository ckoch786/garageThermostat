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
#include "motionControl.h"


// ========== Main Program ==========
int main(void) {
  char buffer[16];
  SHT41_Data sensor_data;

  // Initialize peripherals
  uart_init(9600);
  i2c_init();
  //adc_init();
  analog_comp_init();
  //_delay_ms(100);

  uart_puts("Garage Control Center\r\n");

  hcsr04_init();
  motionControl_init();
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
      uint8_t remaining = DISPLAY_TIMEOUT_SEC - display_cntrl.timer_seconds;
    if (display_cntrl.broadcast || displayOn) {
      display_cntrl.needs_update = 0;  // Clear flag

      if (!displayOn) {
        ssd1306_clear();
      }

      // Read sensor
      if (sht41_read_measurement(&sensor_data)) {

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
        } else {
          ssd1306_clear();
          ssd1306_puts("READ ERROR", 20, 3, 1);
          uart_puts("Measurement failed\r\n");
        }  // End if (!display_cntrl.broadcast) {
      }    // End if (sht41_read_measurement(&sensor_data)) {

      uint32_t distance_cm = hcsr04_read();

      if (distance_cm <= 6) {
        //ssd1306_clear();
        // Display ON on the right bottom of screen
        ssd1306_puts("ON ", 100, 6, 4);
        motionControl_toggleIndicator();
        
        char buffer[32];
        sprintf(buffer, "ON: %lu in\n\r", distance_cm);
        uart_puts(buffer);
      } else {
        ssd1306_puts("OFF", 100, 6, 4);
      }

      // Dislay distance from ultrasonic sensor on right of display
      ssd1306_puts("DIST:", 100, 2, 1);
      sprintf(buffer, "%lu in", distance_cm);
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

    // TODO Low power idle - wait for interrupts
    _delay_ms(50);
  }

  return 0;
}
