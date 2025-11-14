#include "uart.h"

void uart_init(uint32_t baud) {
  uint16_t ubrr = F_CPU / 16 / baud - 1;
  UBRR0H = (uint8_t)(ubrr >> 8);
  UBRR0L = (uint8_t)ubrr;
  UCSR0B = (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_putchar(char c) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = c;
}

void uart_puts(const char *s) {
  while (*s) uart_putchar(*s++);
}

void uart_print_uint16(uint16_t value) {
  char buffer[6];
  uint8_t i = 0;
  do {
    buffer[i++] = '0' + (value % 10);
    value /= 10;
  } while (value > 0);
  while (i > 0) {
    uart_putchar(buffer[--i]);
  }
}

void uart_print_float(float value, uint8_t decimals) {
  int32_t int_part = (int32_t)value;
  float frac_part = value - int_part;

  if (int_part < 0) {
    uart_putchar('-');
    int_part = -int_part;
    frac_part = -frac_part;
  }

  char buffer[10];
  uint8_t i = 0;
  do {
    buffer[i++] = '0' + (int_part % 10);
    int_part /= 10;
  } while (int_part > 0);

  while (i > 0) {
    uart_putchar(buffer[--i]);
  }

  uart_putchar('.');

  for (uint8_t j = 0; j < decimals; j++) {
    frac_part *= 10;
    int32_t digit = (int32_t)frac_part;
    uart_putchar('0' + digit);
    frac_part -= digit;
  }
}

