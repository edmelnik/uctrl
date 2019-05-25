#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include <time.h>


#include "serial.h"
#include "adc.h"

#define DELAY_MS 1000

struct data{
    int index;
    int values[4];
};

// Setup serial output streams for stdio
// TODO move this to a func in serial.h
FILE uart_output = FDEV_SETUP_STREAM(uartPutchar, NULL,
				     _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uartGetchar,
				    _FDEV_SETUP_READ);
FILE uart_io = FDEV_SETUP_STREAM(uartPutchar, uartGetchar,
				 _FDEV_SETUP_RW);

char* structToJSON(struct data d){
    char *output;
    int written;
    output = malloc(200);
    written = snprintf(output, 200, "{\n\t\"tube\": %d\n\t\"pressure\":%d\n\t", 1, d.values[0]);
    written += snprintf(output+written, 200-written, "\"tube\": %d\n\t\"pressure\":%d\n\t",
			2, d.values[1]);
    written += snprintf(output+written, 200-written, "\"tube\": %d\n\t\"pressure\":%d\n\t",
			3, d.values[2]);
    written += snprintf(output+written, 200-written, "\"tube\": %d\n\t\"pressure\":%d\n",
			4, d.values[3]);
    snprintf(output+written, 200-written, "}");
    return output;
}

int main() {    
    uartInit();
    adcInit();
    
    struct data d;
    char *output;
    int read_val;
    
    stdout = &uart_output;
    stdin = &uart_input;
    output = malloc(200);
    
    while(1){
	// loop through analog channels
	for(int i=0; i<4; i++){
	    read_val = readADC(i);
	    d.values[i] = read_val/2;
	}
	output = structToJSON(d);
	puts(output);
	_delay_ms(DELAY_MS);
    }

}
