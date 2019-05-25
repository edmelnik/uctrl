#ifndef ADC_LIB
#define ADC_LIB

#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>

void adcInit(){
    ADMUX = (1<<REFS0);
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
uint16_t readADC(uint8_t ch){
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
    ADCSRA |= (1<<ADSC);
 
    while(ADCSRA & (1<<ADSC)); 
    return (ADC);
}

#endif
