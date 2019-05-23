#include <stdlib.h>
#include "serial.h"
#include "frozen.c"

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
    struct json_out out = JSON_OUT_FILE(stdout);
    
    while(1){
	PORTB |= _BV(PORTB5);
	json_printf(&out, "{%Q: %d, x: [%B, %B], y: %Q}", "foo",
		    123, 0, -1, "hi");
	_delay_ms(PRINT_DELAY_MS);
	PORTB |= _BV(PORTB5);
    }
}
