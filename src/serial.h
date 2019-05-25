
#define F_CPU 16000000UL
#define BAUD 9600

#include <util/setbaud.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

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
	float k = MUX0;
	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait for data reg */
	UDR0 = c;

	/* UDR0 = c; */
	/* loop_until_bit_is_set(UCSR0A, TXC0); /\* wait for Tx *\/ */
}
char uartGetchar(FILE *stream){
    loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
    return UDR0;
}
