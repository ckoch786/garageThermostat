#include "hcsr04.h"

void hcsr04_init(void)
{
  // Trig is output, Echo is input
  DDRB |= (1 << hcsr04_trig_pin);   // Trig as output
  DDRB &= ~(1 << hcsr04_echo_pin);  // Echo as input
  
  // Set trig low initially
  hcsr04_port &= ~(1 << hcsr04_trig_pin);
}

uint32_t hcsr04_read(void)
{
  char buffer[32];
  uart_puts("start read()\n\r");
  // Send 10us trigger pulse
  hcsr04_port &= ~(1 << hcsr04_trig_pin);
  _delay_us(2);
  hcsr04_port |= (1 << hcsr04_trig_pin);
  _delay_us(10);  // 10 microseconds
  hcsr04_port &= ~(1 << hcsr04_trig_pin);

  // Wait for echo to go HIGH
  uint32_t timeout = 0;
  while(!(hcsr04_trig_pin_output & (1 << hcsr04_echo_pin)))
  {
    timeout++;
    if(timeout > TIMEOUT_MAX) {
      return 0; // Timeout protection
    }
  }
  
  // Measure echo pulse width
  uint32_t timer = 0;
  while(hcsr04_trig_pin_output & (1 << hcsr04_echo_pin))
  {
    timer++;
    _delay_us(1);
    if(timer > TIMEOUT_MAX) {
      return 0; // Timeout protection
    }
  }

  sprintf(buffer, "Timer value: %lu\n\r", timer);
  uart_puts(buffer);
  
  // Calculate distance in cm
  // Speed of sound = 343 m/s = 0.0343 cm/us
  // Distance = (time * 0.0343) / 2
  // With _delay_us(1) in loop, timer ≈ microseconds
  uint32_t distance_cm = (timer * 343UL) / 20000UL;
  sprintf(buffer, "Distance: %lu cm\n\r", distance_cm);
  uart_puts(buffer);
  
  return distance_cm;
}