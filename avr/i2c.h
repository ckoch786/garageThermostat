#ifndef I2C_H
#define I2C_H

#include <stdint.h>

// I2C definitions
#define SCL_CLOCK 100000L
#define TWI_FREQ ((F_CPU / SCL_CLOCK) - 16) / 2

// SHT41 I2C address
#define SHT41_ADDR 0x44

// SSD1306 I2C address
#define SSD1306_ADDR 0x3C

// I2C status codes
#define TW_START 0x08
#define TW_REP_START 0x10
#define TW_MT_SLA_ACK 0x18
#define TW_MT_DATA_ACK 0x28
#define TW_MR_SLA_ACK 0x40
#define TW_MR_DATA_ACK 0x50
#define TW_MR_DATA_NACK 0x58

void i2c_init(void);

uint8_t i2c_start(void);

void i2c_stop(void);

uint8_t i2c_write(uint8_t data);

uint8_t i2c_read_ack(void);

uint8_t i2c_read_nack(void);

#endif