/*
Read values from the ADC port and output the values to serial
source: http://maxembedded.com/2011/06/the-adc-of-the-avr/

Should ideally use functions in serial.c for output
*/

#include <stdint.h>
#include <stdlib.h>
/* #include <string.h> */

#include "serial.h"

// next two are included in serial
/* #include <util/setbaud.h> */
/* #include <avr/io.h> */

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

// Setup serial output streams for stdio
FILE uart_output = FDEV_SETUP_STREAM(uartPutchar, NULL,
				     _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uartGetchar,
				    _FDEV_SETUP_READ);
FILE uart_io = FDEV_SETUP_STREAM(uartPutchar, uartGetchar,
				 _FDEV_SETUP_RW);

int main(){
    initADC();
    uartInit();
    
    stdout = &uart_output;
    stdin = &uart_input;
    
    uint16_t adc_read_val;
    char msg[20];
    DDRB |= _BV(DDB5);    
    while(1){
	adc_read_val = readADC(0);
	PORTB |= _BV(PORTB5);
	// For some reason the value is twice actual input voltage
	// Might be a good idea to consult the datasheets at some point
	//  to figure out why
	printf("%d\n", adc_read_val/2); 
	_delay_ms(PRINT_DELAY_MS);
	PORTB |= _BV(PORTB5);
    }

}
