
#define F_CPU 16000000UL
#define BAUD 9600

#include <util/setbaud.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#define PRINT_DELAY_MS 1000

void uartInit();
void uartPutchar(char c, FILE *stream);
char uartGetchar(FILE *stream);

/* FILE uart_output = FDEV_SETUP_STREAM(uartPutchar, NULL, _FDEV_SETUP_WRITE); */
/* FILE uart_input = FDEV_SETUP_STREAM(NULL, uartGetchar, _FDEV_SETUP_READ); */
/* FILE uart_io = FDEV_SETUP_STREAM(uartPutchar, uartGetchar, _FDEV_SETUP_RW); */
