#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <util/setbaud.h>
#include <avr/io.h>

void initADC(){
    ADMUX = (1<<REFS0);
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t readADC(uint8_t ch){
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);
 
    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1<<ADSC)); 
    return (ADC);
}

int main(){
    initADC();
    uint16_t adc_read_val;
    adc_read_val = readADC(0);
    itoa(adc_read_val, msg, 20);
}
