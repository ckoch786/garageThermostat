#include <avr/io.h>
#include <stdint.h>
#include "i2c.h"

// ========== I2C Functions ==========
void i2c_init(void) {
  TWSR = 0x00;
  TWBR = (uint8_t)TWI_FREQ;
  TWCR = (1 << TWEN);
}

uint8_t i2c_start(void) {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));
  return (TWSR & 0xF8);
}

void i2c_stop(void) {
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
  while (TWCR & (1 << TWSTO));
}

uint8_t i2c_write(uint8_t data) {
  TWDR = data;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));
  return (TWSR & 0xF8);
}

uint8_t i2c_read_ack(void) {
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
  while (!(TWCR & (1 << TWINT)));
  return TWDR;
}

uint8_t i2c_read_nack(void) {
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT)));
  return TWDR;
}
