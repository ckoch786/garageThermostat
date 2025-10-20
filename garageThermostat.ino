#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define F_CPU 16000000UL

// I2C definitions
#define SCL_CLOCK 100000L
#define TWI_FREQ ((F_CPU/SCL_CLOCK)-16)/2

// SHT41 I2C address
#define SHT41_ADDR 0x44

// SHT41 commands
#define SHT41_CMD_MEASURE_HIGH_PRECISION 0xFD
#define SHT41_CMD_SOFT_RESET 0x94
#define SHT41_CMD_READ_SERIAL 0x89

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

// I2C Functions
void i2c_init(void) {
    TWSR = 0x00; // Prescaler = 1
    TWBR = (uint8_t)TWI_FREQ;
    TWCR = (1 << TWEN); // Enable TWI
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

// CRC-8 checksum (polynomial 0x31)
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

// SHT41 Functions
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
    
    // Send high precision measurement command
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
    
    // Wait for measurement to complete (high precision: ~8.3ms)
    _delay_ms(10);
    
    // Read 6 bytes: temp_msb, temp_lsb, temp_crc, hum_msb, hum_lsb, hum_crc
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
    
    // Verify temperature CRC
    temp_data[0] = raw_data[0];
    temp_data[1] = raw_data[1];
    if (crc8(temp_data, 2) != raw_data[2]) return 0;
    
    // Verify humidity CRC
    temp_data[0] = raw_data[3];
    temp_data[1] = raw_data[4];
    if (crc8(temp_data, 2) != raw_data[5]) return 0;
    
    // Convert raw values to physical units
    uint16_t temp_raw = (raw_data[0] << 8) | raw_data[1];
    uint16_t hum_raw = (raw_data[3] << 8) | raw_data[4];
    
    // Temperature conversion: -45 + 175 * (raw / 65535)
    data->temperature = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
    
    // Humidity conversion: -6 + 125 * (raw / 65535)
    data->humidity = -6.0f + 125.0f * ((float)hum_raw / 65535.0f);
    
    // Clamp humidity to valid range
    if (data->humidity < 0.0f) data->humidity = 0.0f;
    if (data->humidity > 100.0f) data->humidity = 100.0f;
    
    return 1;
}

// UART functions for output (optional)
void uart_init(uint32_t baud) {
    uint16_t ubrr = F_CPU/16/baud - 1;
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

void uart_print_float(float value, uint8_t decimals) {
    int32_t int_part = (int32_t)value;
    float frac_part = value - int_part;
    
    if (int_part < 0) {
        uart_putchar('-');
        int_part = -int_part;
        frac_part = -frac_part;
    }
    
    // Print integer part
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
    
    // Print fractional part
    for (uint8_t j = 0; j < decimals; j++) {
        frac_part *= 10;
        int32_t digit = (int32_t)frac_part;
        uart_putchar('0' + digit);
        frac_part -= digit;
    }
}

int main(void) {
    SHT41_Data sensor_data;
    
    // Initialize peripherals
    uart_init(9600);
    i2c_init();
    _delay_ms(100);
    
    uart_puts("SHT41 High Precision Sensor\r\n");
    // Soft reset sensor
    
    if (sht41_soft_reset()) {
        uart_puts("Sensor reset OK\r\n");
    } else {
        uart_puts("Sensor reset failed\r\n");
    }
    
    _delay_ms(100);
    
    while (1) {
        if (sht41_read_measurement(&sensor_data)) {
            uart_puts("Temperature: ");
            uart_print_float(sensor_data.temperature, 2);
            uart_puts(" C ");
            
            uart_puts("Humidity: ");
            uart_print_float(sensor_data.humidity, 2);
            uart_puts(" %\r\n\r\n");
        } else {
            uart_puts("Measurement failed\r\n");
        }
        
        _delay_ms(2000);
    }
    
    return 0;
}