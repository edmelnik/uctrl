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

struct pressure_values{
    int p_val[4];
};

struct pressure_values p_struct;

void setup(){
    delay(200);            // my particular board needs this
    Serial.begin(9600);
    Wire.begin();
}

int get_i2c(int mux_addr, int mux_channel, int device_addr, int min, int max, char* pressure_out){
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
	/* Serial.println("FAILED"); */
	strcpy(pressure_out, "ERR");
	return -1;
    }
    else{
	ps_convert(ps, &p, &t, OUTPUT_MIN, OUTPUT_MAX, min, max);
	dtostrf(p, 2, 3, p_str); 
	dtostrf(t, 2, 3, t_str);
	
	/* dtostrf(ps.bridge_data, 2, 2, p_raw_str); */
	/* Serial.println(p_raw_str); */
	/* Serial.println(p_str); */
	
	strcpy(pressure_out, p_str);
	/* p_struct.p_val[mux_channel-1] = p; // indexed by 0 */
	return 1;
    }
        
    
    /* if ((now - prev > interval) && (Serial.available() <= 0)){
    /* 	prev = now; */
    /* 	el = ps_get_raw(device_addr, &ps); */
    /* 	// for some reason my chip triggers a diagnostic fault */
    /* 	// on 50% of powerups without a notable impact */
    /* 	// to the output values. */
    /* 	if ( el == 4 ) { */
    /* 	    Serial.println("err sensor missing"); */
    /* 	} */
    /* 	else { */
    /* 	    if ( el == 3 ) { */
    /* 		Serial.print("err diagnostic fault "); */
    /* 		Serial.println(ps.status, BIN); */
    /* 	    } */
    /* 	    if ( el == 2 ) { */
    /* 		// if data has already been feched since the last */
    /* 		// measurement cycle */
    /* 		Serial.print("warn stale data "); */
    /* 		Serial.println(ps.status, BIN); */
    /* 	    } */
    /* 	    if ( el == 1 ) { */
    /* 		// chip in command mode */
    /* 		// no clue how to end up here */
    /* 		Serial.print("warn command mode "); */
    /* 		Serial.println(ps.status, BIN); */
    /* 	    } */
    /* 	    ps_convert(ps, &p, &t, OUTPUT_MIN, OUTPUT_MAX, min, max); */
    /* 	    // floats cannot be easily printed out */
    /* 	    dtostrf(p, 2, 2, p_str); */
    /* 	    dtostrf(t, 2, 2, t_str); */
	    
    /* 	    Serial.println(p); */
    /* 	    return; */
    /* 	} */
    /* } */
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
	/* Serial.println(pressure); */
	if(retval<0){ // Error
	    /* Serial.print(pressure); */
	    errs[i-1] = retval; // store the error here for now
	}
	else{
	    /* Serial.print(pressure); */
	    // nothing needs to be done, since 
	    // pressure values are in the global struct p_struct
	    /* strcpy(buffer+buf_ptr, *pressure); */
	    /* buf_ptr+=strlen(pressure); */
	    /* Serial.print(pressure); */
	}
	buf_ptr += snprintf(buffer+buf_ptr, 50-buf_ptr,
			    " %s ", pressure);
    }
    /* snprintf(buffer+buf_ptr, 50-buf_ptr, "\n"); */
    Serial.println(buffer);
    /* for(i=0; i<4; i++){ */
    /* 	if(errs[i] == 1) */
    /* 	    buf_ptr += snprintf(buffer, 50-buf_ptr, " %s ", err_str); */
    /* 	else */
    /* 	    buf_ptr += snprintf(buffer, 50-buf_ptr, " %d ", */
    /* 				p_struct.p_val[i]); */
    /* } */
    
    Serial.println(buffer);
    /* get_i2c(MUXADDR, SLAVE0_MUX, SLAVE_ADDR, P_MIN0, P_MAX0); */
    /* get_i2c(MUXADDR, SLAVE1_MUX, SLAVE1_ADDR, P_MIN1, P_MAX1); */
    delay(600);
}
