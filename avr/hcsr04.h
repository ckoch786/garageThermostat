#ifndef HCSR04x_H
#define HCSR04x_H

#include <stdint.h>
#include <avr/io.h>

#define hcsr04_port PORTB
#define hcsr04_trig_pin_output PINB
#define hcsr04_trig_pin 1
#define hcsr04_echo_pin 2
#define TIMEOUT_MAX 30000

void hcsr04_init(void);
uint32_t hcsr04_read(void);

#endif