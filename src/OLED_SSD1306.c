#include "OLED_SSD1306.h"

// ========== SSD1306 OLED Functions ==========
void ssd1306_command(uint8_t cmd) {
  i2c_start();
  i2c_write((SSD1306_ADDR << 1) | 0);
  i2c_write(0x00);  // Command mode
  i2c_write(cmd);
  i2c_stop();
}

void ssd1306_data(uint8_t data) {
  i2c_start();
  i2c_write((SSD1306_ADDR << 1) | 0);
  i2c_write(0x40);  // Data mode
  i2c_write(data);
  i2c_stop();
}

void ssd1306_init(void) {
  _delay_ms(100);
  ssd1306_command(SSD1306_DISPLAYOFF);
  ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
  ssd1306_command(0x80);
  ssd1306_command(SSD1306_SETMULTIPLEX);
  ssd1306_command(0x3F);
  ssd1306_command(SSD1306_SETDISPLAYOFFSET);
  ssd1306_command(0x00);
  ssd1306_command(SSD1306_SETSTARTLINE | 0x00);
  ssd1306_command(SSD1306_CHARGEPUMP);
  ssd1306_command(0x14);
  ssd1306_command(SSD1306_MEMORYMODE);
  ssd1306_command(0x00);
  ssd1306_command(SSD1306_SEGREMAP | 0x01);
  ssd1306_command(SSD1306_COMSCANDEC);
  ssd1306_command(SSD1306_SETCOMPINS);
  ssd1306_command(0x12);
  ssd1306_command(SSD1306_SETCONTRAST);
  ssd1306_command(0xCF);
  ssd1306_command(SSD1306_SETPRECHARGE);
  ssd1306_command(0xF1);
  ssd1306_command(SSD1306_SETVCOMDETECT);
  ssd1306_command(0x40);
  ssd1306_command(SSD1306_DISPLAYALLON_RESUME);
  ssd1306_command(SSD1306_NORMALDISPLAY);
  ssd1306_command(SSD1306_DISPLAYON);
}

void ssd1306_clear(void) {
  for (uint8_t page = 0; page < 8; page++) {
    ssd1306_command(0xB0 + page);
    ssd1306_command(0x00);
    ssd1306_command(0x10);
    for (uint8_t col = 0; col < 128; col++) {
      ssd1306_data(0x00);
    }
  }
}

void ssd1306_set_position(uint8_t x, uint8_t y) {
  ssd1306_command(0xB0 + y);
  ssd1306_command(0x00 + (x & 0x0F));
  ssd1306_command(0x10 + ((x >> 4) & 0x0F));
}

void ssd1306_putchar(char c, uint8_t size) {
  if (c < 32 || c > 90) c = 32;
  for (uint8_t i = 0; i < 5; i++) {
    uint8_t line = pgm_read_byte(&font5x7[c - 32][i]);
    if (size == 1) {
      ssd1306_data(line);
    } else {
      uint8_t expanded = 0;
      for (uint8_t bit = 0; bit < 8; bit++) {
        if (line & (1 << bit)) {
          expanded |= (3 << (bit));
        }
      }
      ssd1306_data(expanded);
      ssd1306_data(expanded);
    }
  }
  ssd1306_data(0x00);
}

void ssd1306_puts(const char *s, uint8_t x, uint8_t y, uint8_t size) {
  ssd1306_set_position(x, y);
  while (*s) {
    ssd1306_putchar(*s++, size);
  }
}

void ssd1306_print_float(float value, uint8_t decimals, uint8_t x, uint8_t y, uint8_t size) {
  char buffer[16];
  uint8_t idx = 0;
  int32_t int_part = (int32_t)value;
  float frac_part = value - int_part;

  if (int_part < 0) {
    buffer[idx++] = '-';
    int_part = -int_part;
    frac_part = -frac_part;
  }

  char temp[10];
  uint8_t i = 0;
  do {
    temp[i++] = '0' + (int_part % 10);
    int_part /= 10;
  } while (int_part > 0);

  while (i > 0) {
    buffer[idx++] = temp[--i];
  }

  buffer[idx++] = '.';

  for (uint8_t j = 0; j < decimals; j++) {
    frac_part *= 10;
    int32_t digit = (int32_t)frac_part;
    buffer[idx++] = '0' + digit;
    frac_part -= digit;
  }

  buffer[idx] = '\0';
  ssd1306_puts(buffer, x, y, size);
}

// ========== CRC-8 Checksum ==========
uint8_t crc8(uint8_t *data, uint8_t len) {
  uint8_t crc = 0xFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80)
        crc = (crc << 1) ^ 0x31;
      else
        crc = crc << 1;
    }
  }
  return crc;
}
