#include "serial.h"
#include "jsmn.h"

// Setup serial output streams for stdio
FILE uart_output = FDEV_SETUP_STREAM(uartPutchar, NULL,
				     _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uartGetchar,
				    _FDEV_SETUP_READ);
FILE uart_io = FDEV_SETUP_STREAM(uartPutchar, uartGetchar,
				 _FDEV_SETUP_RW);

int main(){
    uartInit();

    stdout = &uart_output;
    stdin = &uart_input;
    
    while(1){
	PORTB |= _BV(PORTB5);
	_delay_ms(PRINT_DELAY_MS);
	PORTB |= _BV(PORTB5);
    }
}
