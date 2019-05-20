/*
  Communicating directly with the AVR microcontroller using serial
  Source: https://appelsiini.net/2011/simple-usart-with-avr-libc/

  Hopefully this code will provide a simple library that could be used 
  with pressure sensor I2C communications in the future
*/

#define F_CPU 16000000UL
#define BAUD 9600

#include <util/setbaud.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#define PRINT_DELAY_MS 1000

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

void uartPutchar(char c, FILE *stream){
    /* if(c == '\n') */
    /* 	uartPutchar('\r', stream); */

    loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait for data reg */
    UDR0 = c;

    /* UDR0 = c; */
    /* loop_until_bit_is_set(UCSR0A, TXC0); /\* wait for Tx *\/ */
}

char uartGetchar(FILE *stream){
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}

FILE uart_output = FDEV_SETUP_STREAM(uartPutchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uartGetchar, _FDEV_SETUP_READ);
FILE uart_io = FDEV_SETUP_STREAM(uartPutchar, uartGetchar, _FDEV_SETUP_RW);

int main(){
    uartInit();
    stdout = &uart_output;
    stdin = &uart_input;
    char msg[] = "946093318444555094179583640END";
    DDRB |= _BV(DDB5);
    while(1){
	puts(msg);
	PORTB |= _BV(PORTB5);
	_delay_ms(PRINT_DELAY_MS);
	PORTB |= _BV(PORTB5);
	/* printf(msg); */
    }
}
