#ifndef OLED_SSD1306_H
#define OLED_SSD1306_H

#include <stdint.h>
#include <avr/pgmspace.h>

// Function prototypes for OLED/SSD1306 implementation
void ssd1306_command(uint8_t cmd);
void ssd1306_data(uint8_t data);
void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_set_position(uint8_t x, uint8_t y);
void ssd1306_putchar(char c, uint8_t size);
void ssd1306_puts(const char *s, uint8_t x, uint8_t y, uint8_t size);
void ssd1306_print_float(float value, uint8_t decimals, uint8_t x, uint8_t y, uint8_t size);

extern const uint8_t font5x7[][5];


// SSD1306 commands
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR 0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D

#endif