
#include <stdint.h>
#include <stdlib.h>

/* also included in serial: will this fail? */

/* #include <util/setbaud.h> */
#include <avr/io.h>

void adcInit();
uint16_t readADC(uint8_t ch);
