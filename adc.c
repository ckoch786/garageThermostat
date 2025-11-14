#include "adc.h"

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

