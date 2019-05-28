/*
  this is the one for one
*/
#include <Wire.h>
#include <string.h>
#include "hsc_ssc_i2c.h"

#define MUXADDR 0x70

// see hsc_ssc_i2c.h for a description of these values
// these defaults are valid for the HSCMRNN030PA2A3 chip
#define SLAVE0_MUX 2
#define SLAVE_ADDR 0x28
#define SLAVE1_MUX 3
#define SLAVE1_ADDR 0x28
#define OUTPUT_MIN 0
#define OUTPUT_MAX 0x3fff       // 2^14 - 1
/* #define P_MIN0 -27.07        // min is 0 for sensors that give absolute values */
/* #define P_MAX0 27.07   // 1psi (and we want results in pascals) */
/* #define P_MIN1 -135.35 */
/* #define P_MAX1 135.35 */

// Following values are valid for the SSC PD001/PD005 (5V) sensors currently in use
#define P_MIN0 -1
#define P_MAX0 1
#define P_MIN1 -5
#define P_MAX1 5

uint32_t prev = 0;
const uint32_t interval = 10;

void setup(){
    delay(200);            // my particular board needs this
    Serial.begin(9600);
    Wire.begin();
}

int get_i2c(int mux_addr, int mux_channel, int device_addr,
	    int min, int max, char* pressure_out){

    char errstr[20] = "ERR", *errval;
    errval = malloc(3);
    
    Wire.beginTransmission(mux_addr);
    Wire.write(1 << mux_channel);
    Wire.endTransmission();

    unsigned long now = millis();
    struct cs_raw ps;
    char p_str[10], t_str[10], p_raw_str[10];
    uint8_t el;
    float p, t;
    el = ps_get_raw(device_addr, &ps);
    if(el <= 4 && el >= 1){
	strcat(errstr, itoa(el, errval, 10));
	strcpy(pressure_out, errstr);
	return -1;
    }
    else{
	ps_convert(ps, &p, &t, OUTPUT_MIN, OUTPUT_MAX, min, max);
	dtostrf(p, 2, 3, p_str); 
	dtostrf(t, 2, 3, t_str);	
	strcpy(pressure_out, p_str);
	return 1;
    }           
}

void loop(){
    char *buffer, *pressure, err_str[] = " ERR ";
    int errs[4] = {0, 0, 0, 0};
    int i, retval, buf_ptr = 0;

    buffer = malloc(50);

    pressure = malloc(10);
    // indexed by 1 to correspond with mux_channel
    for(i=1; i<5; i++){
	retval = get_i2c(MUXADDR, i, SLAVE_ADDR,
			 P_MIN0, P_MAX0, pressure);
	buf_ptr += snprintf(buffer+buf_ptr, 50-buf_ptr,
			    " %s ", pressure);
    }
    Serial.println(buffer);
    delay(600);
}
