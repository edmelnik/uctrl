/*
  Communicating directly with the AVR microcontroller using serial
  Source: https://appelsiini.net/2011/simple-usart-with-avr-libc/

  Hopefully this code will provide a simple library that could be used 
   with pressure sensor I2C communications in the future
*/

#define F_CPU 16000000UL
#define BAUD 9600
#include <util/setbaud.h>

void uartInit(){
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void uartPutchar(char c) {
    UDR0 = c;
    loop_until_bit_is_set(UCSR0A, TXC0); /* Wait until transmission ready. */
}

char uartGetchar(void) {
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}

