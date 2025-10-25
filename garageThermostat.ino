#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL

// Configuration
#define CELCIUS 0  // 0 for Fahrenheit, 1 for Celsius

// Light sensor on ADC0 (PC0)
#define LIGHT_SENSOR_PIN 0
#define LIGHT_THRESHOLD 307    // ~1.5V threshold for display trigger

// Timer configuration for 30 second timeout
#define TIMER_PRESCALER 1024
#define DISPLAY_TIMEOUT_SEC 30

// I2C definitions
#define SCL_CLOCK 100000L
#define TWI_FREQ ((F_CPU / SCL_CLOCK) - 16) / 2

// SHT41 I2C address
#define SHT41_ADDR 0x44

// SSD1306 I2C address
#define SSD1306_ADDR 0x3C

// SHT41 commands
#define SHT41_CMD_MEASURE_HIGH_PRECISION 0xFD
#define SHT41_CMD_SOFT_RESET 0x94

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

// I2C status codes
#define TW_START 0x08
#define TW_REP_START 0x10
#define TW_MT_SLA_ACK 0x18
#define TW_MT_DATA_ACK 0x28
#define TW_MR_SLA_ACK 0x40
#define TW_MR_DATA_ACK 0x50
#define TW_MR_DATA_NACK 0x58

typedef struct {
  float temperature;
  float humidity;
} SHT41_Data;

// Global state variables
volatile uint8_t display_on = 0;
volatile uint16_t timer_seconds = 0;
volatile uint8_t needs_update = 0;  // Flag for main loop

// 5x7 font (stored in program memory)
const uint8_t font5x7[][5] PROGMEM = {
  { 0x00, 0x00, 0x00, 0x00, 0x00 },  // space
  { 0x00, 0x00, 0x5F, 0x00, 0x00 },  // !
  { 0x00, 0x07, 0x00, 0x07, 0x00 },  // "
  { 0x14, 0x7F, 0x14, 0x7F, 0x14 },  // #
  { 0x24, 0x2A, 0x7F, 0x2A, 0x12 },  // $
  { 0x23, 0x13, 0x08, 0x64, 0x62 },  // %
  { 0x36, 0x49, 0x55, 0x22, 0x50 },  // &
  { 0x00, 0x05, 0x03, 0x00, 0x00 },  // '
  { 0x00, 0x1C, 0x22, 0x41, 0x00 },  // (
  { 0x00, 0x41, 0x22, 0x1C, 0x00 },  // )
  { 0x14, 0x08, 0x3E, 0x08, 0x14 },  // *
  { 0x08, 0x08, 0x3E, 0x08, 0x08 },  // +
  { 0x00, 0x50, 0x30, 0x00, 0x00 },  // ,
  { 0x08, 0x08, 0x08, 0x08, 0x08 },  // -
  { 0x00, 0x60, 0x60, 0x00, 0x00 },  // .
  { 0x20, 0x10, 0x08, 0x04, 0x02 },  // /
  { 0x3E, 0x51, 0x49, 0x45, 0x3E },  // 0
  { 0x00, 0x42, 0x7F, 0x40, 0x00 },  // 1
  { 0x42, 0x61, 0x51, 0x49, 0x46 },  // 2
  { 0x21, 0x41, 0x45, 0x4B, 0x31 },  // 3
  { 0x18, 0x14, 0x12, 0x7F, 0x10 },  // 4
  { 0x27, 0x45, 0x45, 0x45, 0x39 },  // 5
  { 0x3C, 0x4A, 0x49, 0x49, 0x30 },  // 6
  { 0x01, 0x71, 0x09, 0x05, 0x03 },  // 7
  { 0x36, 0x49, 0x49, 0x49, 0x36 },  // 8
  { 0x06, 0x49, 0x49, 0x29, 0x1E },  // 9
  { 0x00, 0x36, 0x36, 0x00, 0x00 },  // :
  { 0x00, 0x56, 0x36, 0x00, 0x00 },  // ;
  { 0x08, 0x14, 0x22, 0x41, 0x00 },  // <
  { 0x14, 0x14, 0x14, 0x14, 0x14 },  // =
  { 0x00, 0x41, 0x22, 0x14, 0x08 },  // >
  { 0x02, 0x01, 0x51, 0x09, 0x06 },  // ?
  { 0x32, 0x49, 0x79, 0x41, 0x3E },  // @
  { 0x7E, 0x11, 0x11, 0x11, 0x7E },  // A
  { 0x7F, 0x49, 0x49, 0x49, 0x36 },  // B
  { 0x3E, 0x41, 0x41, 0x41, 0x22 },  // C
  { 0x7F, 0x41, 0x41, 0x22, 0x1C },  // D
  { 0x7F, 0x49, 0x49, 0x49, 0x41 },  // E
  { 0x7F, 0x09, 0x09, 0x09, 0x01 },  // F
  { 0x3E, 0x41, 0x49, 0x49, 0x7A },  // G
  { 0x7F, 0x08, 0x08, 0x08, 0x7F },  // H
  { 0x00, 0x41, 0x7F, 0x41, 0x00 },  // I
  { 0x20, 0x40, 0x41, 0x3F, 0x01 },  // J
  { 0x7F, 0x08, 0x14, 0x22, 0x41 },  // K
  { 0x7F, 0x40, 0x40, 0x40, 0x40 },  // L
  { 0x7F, 0x02, 0x0C, 0x02, 0x7F },  // M
  { 0x7F, 0x04, 0x08, 0x10, 0x7F },  // N
  { 0x3E, 0x41, 0x41, 0x41, 0x3E },  // O
  { 0x7F, 0x09, 0x09, 0x09, 0x06 },  // P
  { 0x3E, 0x41, 0x51, 0x21, 0x5E },  // Q
  { 0x7F, 0x09, 0x19, 0x29, 0x46 },  // R
  { 0x46, 0x49, 0x49, 0x49, 0x31 },  // S
  { 0x01, 0x01, 0x7F, 0x01, 0x01 },  // T
  { 0x3F, 0x40, 0x40, 0x40, 0x3F },  // U
  { 0x1F, 0x20, 0x40, 0x20, 0x1F },  // V
  { 0x3F, 0x40, 0x38, 0x40, 0x3F },  // W
  { 0x63, 0x14, 0x08, 0x14, 0x63 },  // X
  { 0x07, 0x08, 0x70, 0x08, 0x07 },  // Y
  { 0x61, 0x51, 0x49, 0x45, 0x43 },  // Z
};

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

// ========== UART Functions ==========
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

// ========== ADC Functions ==========
void adc_init(void) {
  ADMUX = (1 << REFS0);  // AVCC reference
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  // Enable, prescaler 128
  
  // Dummy conversion for initialization
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
}

uint16_t adc_read(uint8_t channel) {
  ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
  _delay_us(10);  // Allow mux to settle
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADC;
}

// ========== Display Control Functions ==========
void enable_display(void) {
  display_on = 1;
  timer_seconds = 0;
  TCNT1 = 0;
  needs_update = 1;  // Signal main loop to update
}

void disable_display(void) {
  if (display_on) {
    display_on = 0;
    timer_seconds = 0;
    TCNT1 = 0;
    ssd1306_clear();  // Clear when turning off
  }
}

// ========== Timer Setup ==========
void init_timer1(void) {
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC mode, prescaler 1024
  OCR1A = (F_CPU / TIMER_PRESCALER) - 1;  // ~15624 for 1 second
  TIMSK1 = (1 << OCIE1A);  // Enable compare match interrupt
}

// ========== Pin Change Interrupt Setup ==========
void init_light_sensor_interrupt(void) {
  DDRC &= ~(1 << PC0);     // PC0 as input
  PCMSK1 |= (1 << PCINT8); // Enable PCINT8 (PC0)
  PCICR |= (1 << PCIE1);   // Enable PCINT1 interrupt group
}

// ========== ISRs ==========

// Pin Change Interrupt - triggers on light sensor activity
ISR(PCINT1_vect) {
  // Check if PC0 is HIGH (light detected)
  if (PINC & (1 << PC0)) {
    // Read ADC to confirm threshold
    uint16_t sensor_value = adc_read(LIGHT_SENSOR_PIN);
    if (sensor_value > LIGHT_THRESHOLD) {
      enable_display();
    }
  }
}

// Timer1 Compare Match - called every second
ISR(TIMER1_COMPA_vect) {
  if (display_on) {
    timer_seconds++;
    if (timer_seconds >= DISPLAY_TIMEOUT_SEC) {
      disable_display();
    } else {
      needs_update = 1;  // Update display every second
    }
  }
}

// ========== Main Program ==========
int main(void) {
  SHT41_Data sensor_data;

  // Initialize peripherals
  uart_init(9600);
  i2c_init();
  adc_init();
  _delay_ms(100);

  uart_puts("SHT41 Sensor with OLED Display\r\n");

  // Initialize OLED
  ssd1306_init();
  ssd1306_clear();

  // Display title
  ssd1306_puts("SHT41 SENSOR", 10, 0, 1);

  // Soft reset sensor
  if (sht41_soft_reset()) {
    ssd1306_puts("INIT OK", 30, 1, 1);
    uart_puts("Sensor reset OK\r\n");
  } else {
    ssd1306_puts("INIT FAIL", 25, 1, 1);
    uart_puts("Sensor reset failed\r\n");
  }

  _delay_ms(2000);
  ssd1306_clear();

  // Initialize interrupts
  init_light_sensor_interrupt();
  init_timer1();
  
  // Enable global interrupts
  sei();

  uart_puts("System initialized. Waiting for light trigger...\r\n");

  // Main loop
  while (1) {
    // Only update display when flag is set by interrupt
    //if (needs_update && display_on) {
    if(1) {
      needs_update = 0;  // Clear flag
      
      // Read sensor
      if (sht41_read_measurement(&sensor_data)) {
        // Clear and update display
        ssd1306_clear();

        // Display temperature
        ssd1306_puts("TEMP:", 0, 1, 1);
        ssd1306_print_float(sensor_data.temperature, 1, 0, 2, 1);
#if CELCIUS
        ssd1306_puts("C", 50, 2, 1);
#else
        ssd1306_puts("F", 50, 2, 1);
#endif

        // Display humidity
        ssd1306_puts("HUMID:", 0, 4, 1);
        ssd1306_print_float(sensor_data.humidity, 1, 0, 5, 1);
        ssd1306_puts("%", 50, 5, 1);

        // Display timeout countdown
        uint8_t remaining = DISPLAY_TIMEOUT_SEC - timer_seconds;
        ssd1306_puts("TIME:", 0, 7, 1);
        if (remaining < 10) {
          ssd1306_putchar('0' + remaining, 1);
        } else {
          ssd1306_putchar('0' + (remaining / 10), 1);
          ssd1306_putchar('0' + (remaining % 10), 1);
        }

        // UART output
        uart_puts("Temp: ");
        uart_print_float(sensor_data.temperature, 1);
#if CELCIUS
        uart_puts("C  Humid: ");
#else
        uart_puts("F  Humid: ");
#endif
        uart_print_float(sensor_data.humidity, 1);
        uart_puts("%  Time: ");
        uart_print_uint16(remaining);
        uart_puts("s\r\n");
      } else {
        ssd1306_clear();
        ssd1306_puts("READ ERROR", 20, 3, 1);
        uart_puts("Measurement failed\r\n");
      }
    }
    
    // Low power idle - wait for interrupts
    _delay_ms(50);
  }

  return 0;
}