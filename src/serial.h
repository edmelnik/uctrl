
#define F_CPU 16000000UL
#define BAUD 9600

#include <util/setbaud.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>



void uartInit();
void uartPutchar(char c, FILE *stream);
char uartGetchar(FILE *stream);
