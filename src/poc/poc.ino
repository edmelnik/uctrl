
#include <ModbusMaster.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <string.h>
#include "hsc_ssc_i2c.h"

#define NUM_SENSORS 4
#define BAUD 9600

// SST Registers
// Input regs 
#define O2AVG_REG  30001
#define STATUS_REG 30004
#define ERR_REG    30005
#define CALSTS_REG 30018
// Holding regs
#define ONOFF_REG   40001
#define CLCTRL_REG  40004 // Calibration control
#define ADDR_REG    40006
#define ERR_CLR_REG 40002

#define MAX_SENSORS 6

// Constant mux address
#define MUXADDR 0x70

// see hsc_ssc_i2c.h for a description of these values
// these defaults are valid for the HSCMRNN030PA2A3 chip
#define SLAVE_ADDR 0x28
#define OUTPUT_MIN 0
#define OUTPUT_MAX 0x3fff       // 2^14 - 1
#define P_MIN0 -1
#define P_MAX0 1
#define P_MIN1 -5
#define P_MAX1 5

// Starting PIN on the microcontroller
const static unsigned int START_PIN PROGMEM = 4;
// Delay for periodic checks (status, calibration et cetera)
const static unsigned int CHK_DELAY = 20;

// System status
const static int IDLE     PROGMEM = 0;
const static int STARTUP  PROGMEM = 1;
const static int ON       PROGMEM = 2;
const static int SHUTDOWN PROGMEM = 3;
const static int STANDBY  PROGMEM = 4;

// Calibration status codes
const static int CAL_IDLE PROGMEM = 0;
const static int CAL_PROG PROGMEM = 1;
const static int CAL_DONE PROGMEM = 2;

// Flag codes
const static int FLAG_NONE PROGMEM = 0;
const static int FLAG_CAL  PROGMEM = 1;
const static int FLAG_DONE PROGMEM = 2;
const static int FLAG_OFF  PROGMEM = 3;

uint32_t prev = 0;
const uint32_t interval = 10;

SoftwareSerial modbus(4, 5);
ModbusMaster *node = malloc(sizeof(ModbusMaster)*4);

/*
  Status < 0 means there's an active error code in err[] for the sensor
  Error == 0 means that there's sensor status value = valid

  flag[] tracks the calibration demands from the interrupt pins,
  while cal tracks the actual calibration state of the boards
  flag[i] == FLAG_CAL means that calibration is in process for sensor i
  this calibration could be DONE, PROG or IDLE.. cal[i] would hold
  that info
*/
int status[NUM_SENSORS], cal[NUM_SENSORS], flag[NUM_SENSORS] = {FLAG_NONE};

/*

  DONE parse protocol errors

  Response codes

  0  Success
  1 invalid response slave ID exception
  2 invalid response function exception
  3 response timed out exception
  4 invalid response CRC exception
  5 modbus proto illegal func
  6 modbus proto illegal data address
  7 modbus proto illegal data value
  8 modbus proto device failure

  4-20ma.io/ModbusMaster/group__constant.html#gaf69c1c360b84adc5bf1c7bbe071f1970

  This func expects int representations of hex error values and converts them 
  into errors in [-1, -8]
*/
int parseError(int e){
    int ret;
    if(e >= 224 && e <= 227)
	ret = 0-(e % 223);
    else if(e >=1 && e <= 4)
	ret = 0-(e+4);
    return ret;
}

int readReg(int sensor, int reg){
    int res;
    int val;
    res = node[sensor].readInputRegisters(reg, 1);
    if(res == 0){
	val = node[sensor].getResponseBuffer(0);
	node[sensor].clearResponseBuffer();
    }
    else{
	return parseError(res);
    }
    return val;
}

// If this func is taking an int parameter for value, can it be used for calibration?
int writeReg(int sensor, int reg, int value){
    int res, ret;
    res = node[sensor].writeSingleRegister(reg, value);
    if(res == 0)
	ret = res;
    else
	ret = parseError(res);
    return ret;
}

void handleSensor(int i){
    int res, cal_res;
    status[i] = readReg(i, STATUS_REG); delay(4);
    cal[i] = readReg(i, CALSTS_REG); delay(4);
    
    if(flag[i] == FLAG_CAL && cal[i] == CAL_IDLE){ // Calibrate
	res = writeReg(i, CLCTRL_REG, 1); delay(4);
	if(res < 0)
	    status[i] = res;
	else
	    cal[i] = readReg(i, CALSTS_REG); delay(4);
    }
    else if(cal[i] == CAL_DONE){ // Reset sensor
	res = writeReg(i, CLCTRL_REG, 2); delay(4);
	if(res < 0)
	    status[i] = res;
	else{
	    cal[i] = readReg(i, CALSTS_REG); delay(10);	    
	    flag[i] = cal[i];
	}
    }
    else if(cal[i] == CAL_IDLE && flag[i] == FLAG_DONE){ // Reset flag
	flag[i] = 0;
    }
    else if(flag[i] == FLAG_OFF){ // Manual Shutdown
	res = writeReg(i, ONOFF_REG, 0); delay(4);
	if(res < 0)
	    status[i] = res;
	else
	    status[i] = readReg(i, STATUS_REG);
    }
    else if((status[i] == IDLE || status[i] == STANDBY) && flag[i] == FLAG_NONE){ // Turn ON
	res = writeReg(i, ONOFF_REG, 1); delay(4);
	if(res < 0)
	    status[i] = res;
	else
	    status[i] = readReg(i, STATUS_REG); // TODO clear errors
    }    
}

/*

  Data, error, status, calibration output handler used by loop() and calibrate()
  Cal out is a boolean: 1 if called by calibration and only prints cal status (does not set data)

*/
int getVal(int sensor, char *output, unsigned int cal_out=0){
    int retval, data;
    char o2_str[10], errstr[10] = "ERR", *errval, statstr[10] = "STS";
    char calstr[10] = "CAL";
       
    errval = malloc(5);
    
    if(flag[sensor]>FLAG_NONE && flag[sensor]<FLAG_OFF ){ // Get calibration status
	strcat(calstr, itoa(cal[sensor], errval, 10));
	strcpy(output, calstr);
	retval = 1;
    }
    else if(status[sensor] == ON){    // Get O2 data
	data = readReg(sensor, O2AVG_REG);
	if(data < 0){                 // Failed to get data, output ERR
	    strcat(errstr, itoa(data*-1, errval, 10));
	    strcpy(output, errstr);	    
	    retval = -1;
	}
	else{                         // Got data
	    dtostrf(data, 4, 0, o2_str);
	    strcpy(output, o2_str);
	    retval = 1;
	}
    }
    else{
	if(status[sensor] >= 0){     // Status is not ON - get status
	    strcat(statstr, itoa(status[sensor], errval, 10));
	    strcpy(output, statstr);
	    // Return failure only if sensor was not manually off
	    if(flag[sensor] == FLAG_OFF)
		retval = 1;
	    else
		retval = -2;
	}
	else{                       // Error status
	    strcat(errstr, itoa(status[sensor]*-1, errval, 10));
	    strcpy(output, errstr);
	    retval = -3;
	}
    }
    free(errval);
    return retval;
}

int handleCommands(){
    int cmd = 0, count = 0, i, garbage;
    for(i=0; i<8 && Serial.available()>0; i++){
	cmd = Serial.read();
	if(i<=3){ // Handle power commands
	    if(cmd == 48)
		flag[i] = FLAG_OFF;
	    else if(cmd == 49)
		flag[i] = FLAG_NONE;
	}
	else if(i>3){ // Handle calibration commands
	    if(cmd == 49 && flag[i%4] != FLAG_OFF)
		flag[i%4] = FLAG_CAL;
	}
    }
    while(Serial.available() > 0 && count++<30)
    	garbage = Serial.read();
    return 1;
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
	free(errval);
	return -1;
    }
    else{
	ps_convert(ps, &p, &t, OUTPUT_MIN, OUTPUT_MAX, min, max);
	dtostrf(p, 2, 3, p_str); 
	dtostrf(t, 2, 3, t_str);	
	strcpy(pressure_out, p_str);
	free(errval);
	return 1;
    }           
}

void setup(){    
    int i, curr_pin, SST_addr;
    
    Serial.setTimeout(2000);
    Serial.begin(BAUD); // To USB output
    modbus.begin(BAUD); // To RS485 bus
    Wire.begin();
    
    // Actual sensor addresses are indexed by 1, but
    // once sensor addresses are configured in node[], they should indexed by 0
    for(i=0; i<NUM_SENSORS; i++){
    	SST_addr = i+1;
	node[i] = *(new ModbusMaster);
	node[i].begin(SST_addr, modbus);
    }
    
    // If sensor is idle, turn it on
    for(i=0; i<NUM_SENSORS; i++)
	handleSensor(i);

    pinMode(12, INPUT_PULLUP);
    // At this point ideally all sensors are turned on.
    // Sensors that have an active error code and not turned on
    //  should be handled appropriately in loop()
}

int k=0, curr_iter = 0;

void loop(){
    char *buffer;
    
    if(curr_iter == 0){ // Oxygen data
	int i, buf_ptr = 0, data, retval, handle_flag[] = {0,0,0,0};    
	char *output;    

	buffer = malloc(50);
	buf_ptr += snprintf(buffer+buf_ptr, 50-buf_ptr, "O ");
	
	for(i=0; i<NUM_SENSORS; i++){
	    output = malloc(20);
	    if(flag[i] == FLAG_OFF && status[i] == ON){ // Flagged for manual shutdown
		retval = getVal(i, output, 0);
		handle_flag[i] = 1;
	    }
	    if(flag[i] > FLAG_NONE && flag[i] < FLAG_OFF){ // Flagged for calibration
		retval = getVal(i, output, 1);
		handle_flag[i] = 1;
	    }
	    else{
		retval = getVal(i, output, 0); // Not flagged; get data
		if(retval < 0)
		    handle_flag[i] = 1;
	    }
	
	    buf_ptr += snprintf(buffer+buf_ptr, 50-buf_ptr,
				" %s ", output);
	    free(output);
	    delay(4);
	}
	Serial.println(buffer);
	free(buffer);
    
	if(k==CHK_DELAY && Serial.available()){
	    handleCommands();
	    for(i=0; i< NUM_SENSORS; i++)
		if(flag[i] == FLAG_CAL || flag[i] == FLAG_OFF)
		    handle_flag[i] = 1;	
	}
    
	for(i=0; k==CHK_DELAY && i<NUM_SENSORS; i++)
	    if(handle_flag[i] == 1)
		handleSensor(i);
        k+=1;
	k%=(CHK_DELAY+1);
    }
    else if(curr_iter == 1){
	char *pressure;
	int i, retval, buf_ptr = 0;

	buffer = malloc(50);
	pressure = malloc(10);
    	buf_ptr += snprintf(buffer+buf_ptr, 50-buf_ptr, "P ");
	// indexed by 1 to correspond with mux_channel
	for(i=1; i<MAX_SENSORS; i++){
	    retval = get_i2c(MUXADDR, i, SLAVE_ADDR,
			     P_MIN1, P_MAX1, pressure);
	    buf_ptr += snprintf(buffer+buf_ptr, 50-buf_ptr,
				" %s ", pressure);
	}
	Serial.println(buffer);
	Serial.flush();
	free(buffer);
	free(pressure);
    }
    curr_iter = ((curr_iter+1)%2);
}

