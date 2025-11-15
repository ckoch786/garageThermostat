#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(uint32_t baud);

void uart_putchar(char c);

void uart_puts(const char *s);

void uart_print_uint16(uint16_t value);

void uart_print_float(float value, uint8_t decimals);

#endif