#include <stdint.h>
#include "i2c.h"
#include <util/delay.h>
#include "temperature_SHT41.h"

// Forward declaration of crc8 function
uint8_t crc8(uint8_t *data, uint8_t len);

// ========== SHT41 Functions ==========
uint8_t sht41_soft_reset(void) {
  if (i2c_start() != TW_START) return 0;
  if (i2c_write((SHT41_ADDR << 1) | 0) != TW_MT_SLA_ACK) {
    i2c_stop();
    return 0;
  }
  if (i2c_write(SHT41_CMD_SOFT_RESET) != TW_MT_DATA_ACK) {
    i2c_stop();
    return 0;
  }
  i2c_stop();
  _delay_ms(1);
  return 1;
}

uint8_t sht41_read_measurement(SHT41_Data *data) {
  uint8_t raw_data[6];
  uint8_t temp_data[2];

  if (i2c_start() != TW_START) return 0;
  if (i2c_write((SHT41_ADDR << 1) | 0) != TW_MT_SLA_ACK) {
    i2c_stop();
    return 0;
  }
  if (i2c_write(SHT41_CMD_MEASURE_HIGH_PRECISION) != TW_MT_DATA_ACK) {
    i2c_stop();
    return 0;
  }
  i2c_stop();

  _delay_ms(10);

  if (i2c_start() != TW_START) return 0;
  if (i2c_write((SHT41_ADDR << 1) | 1) != TW_MR_SLA_ACK) {
    i2c_stop();
    return 0;
  }

  for (uint8_t i = 0; i < 5; i++) {
    raw_data[i] = i2c_read_ack();
  }
  raw_data[5] = i2c_read_nack();
  i2c_stop();

  temp_data[0] = raw_data[0];
  temp_data[1] = raw_data[1];
  if (crc8(temp_data, 2) != raw_data[2]) return 0;

  temp_data[0] = raw_data[3];
  temp_data[1] = raw_data[4];
  if (crc8(temp_data, 2) != raw_data[5]) return 0;

  uint16_t temp_raw = (raw_data[0] << 8) | raw_data[1];
  uint16_t hum_raw = (raw_data[3] << 8) | raw_data[4];
  data->humidity = -6.0f + 125.0f * ((float)hum_raw / 65535.0f);
  
#if CELCIUS
  data->temperature = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
#else
  data->temperature = -49.0f + 315.0f * ((float)temp_raw / 65535.0f);
#endif

  if (data->humidity < 0.0f) data->humidity = 0.0f;
  if (data->humidity > 100.0f) data->humidity = 100.0f;

  return 1;
}

