#ifndef TEMPERATURE_SHT41_H
#define TEMPERATURE_SHT41_H

typedef struct {
  float temperature;
  float humidity;
} SHT41_Data;

// SHT41 commands
#define SHT41_CMD_MEASURE_HIGH_PRECISION 0xFD
#define SHT41_CMD_SOFT_RESET 0x94

uint8_t sht41_soft_reset(void); 

uint8_t sht41_read_measurement(SHT41_Data *data);

#endif