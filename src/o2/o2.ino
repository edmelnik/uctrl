/*

TODO Better error checking and overall control flow
  - Periodically monitor status and check expected O2 values
  - O2 values could be modelled to make intelligent calibration decisions
TODO Format the output values in a similar manner to how the pressure is being formatted
  - Might be possible to use the output function as a header file common to both
TODO calibration routine
TODO When to use STANDBY mode on sensors?
TODO Read multiple registers and parse the response buffer
  - Select relevent values (see comment above func loop)
TODO Combine o2 and pressure into a single grand PoC

Will the ~ PWM pins with this? Should any strange and unexpected errors occur in the near future, blame this
*/

#include <ModbusMaster.h>
#include <SoftwareSerial.h>
#include <string.h>

#define NUM_SENSORS 4
#define BAUD 9600

// SST Registers
// Input regs 
#define O2AVG_REG  30001
#define STATUS_REG 30004
#define ERR_REG    30005
// Holding regs
#define ONOFF_REG  40001
#define CLCTRL_REG 40004 // Calibration control
#define ADDR_REG   40006

// Starting PIN on the microcontroller
const static int START_PIN PROGMEM = 4;

// System status
const static int IDLE     PROGMEM = 0;
const static int STARTUP  PROGMEM = 1;
const static int ON       PROGMEM = 2;
const static int SHUTDOWN PROGMEM = 3;
const static int STANDBY  PROGMEM = 4;

SoftwareSerial modbus[NUM_SENSORS] = {
    SoftwareSerial(START_PIN, START_PIN+1),
    SoftwareSerial(START_PIN+2, START_PIN+3),
    SoftwareSerial(START_PIN+4, START_PIN+5),
    SoftwareSerial(START_PIN+6, START_PIN+7),
};

// SoftwareSerial *modbus = malloc(sizeof(SoftwareSerial)*4);

// ModbusMaster node[NUM_SENSORS];

ModbusMaster *node =malloc(sizeof(ModbusMaster)*4);

// Status < 0 means there's an active error code in err[] for the sensor
// Error == 0 means that there's sensor status value = valid
int status[NUM_SENSORS], response[NUM_SENSORS], err[NUM_SENSORS];

/*

DONE parse protocol errors

Response codes
0  Success
-1 invalid response slave ID exception
-2 invalid response function exception
-3 response timed out exception
-4 invalid response CRC exception

4-20ma.io/ModbusMaster/group__constant.html#gaf69c1c360b84adc5bf1c7bbe071f1970

This func expects int representations of hex error values and converts them 
into errors in [-1, -4]
*/
int parseError(float e){
    int ret;
    if(e == 0)
	ret = e;
    else
	ret = 0-((int)e % 223);
    return ret;
}

float readReg(int sensor, int reg){
    int res;
    float val;
    modbus[sensor].begin(BAUD);
    res = node[sensor].readInputRegisters(reg, 1);
    if(res == 0){
	val = node[sensor].getResponseBuffer(0);
	node[sensor].clearResponseBuffer();
    }
    else
	return parseError(res);
    return val;    
}

// If this func is taking an int parameter for value, can it be used for calibration?
int writeReg(int sensor, int reg, int value){
    int res;
    modbus[sensor].begin(BAUD);
    res = node[sensor].writeSingleRegister(reg, value);
    return parseError(res);
}

void setup(){    
    int i, curr_pin, SST_addr;    
    Serial.begin(BAUD); // To USB output
   
    // Actual sensor addresses are indexed by 1, but
    // once sensor addresses are configured in node[], they should indexed by 0
    for(i=0; i<NUM_SENSORS; i++){
    	SST_addr = i+1;
    	modbus[i].begin(BAUD);
	node[i] = *(new ModbusMaster);
    	node[i].begin(SST_addr, modbus[i]);
    }
    
    // Get status
    for(i=0; i<NUM_SENSORS; i++){
	modbus[i].begin(BAUD);
	status[i] = readReg(i, STATUS_REG);
	err[i] = status[i];
    }
    
    // If sensor is idle, turn it on
    for(i=0; i<NUM_SENSORS; i++){
	modbus[i].begin(BAUD);
	if(status[i] == IDLE || status[i] == STANDBY)
	    err[i] = writeReg(i, ONOFF_REG, 1);
    }

    // At this point ideally all sensors are turned on.
    // Sensors that have an active error code and not turned on
    //  should be handled appropriately in loop()
}

// TODO Output format
// - STATUS CAL O2%avg O2%raw ERR Heater_voltage ppO2real Pressure PSTemp
// - 30004 30018 30001 30002  30005 30006        30014    30016    30017
// - outputting just o2avg for now

// Every 10 loops, check for status to make sure sensor is on and no errors being sent
// TODO This should ideally be based on a timer instead of looops

int k = 0;

Void handleSensor(int i){
    status[i] = readReg(i, STATUS_REG);
    err[i] = status[i];
    if(status[i] == IDLE || status[i] == STANDBY)
	err[i] = writeReg(i, ONOFF_REG, 1);
}

void loop(){
    int i, buf_ptr = 0;
    float data;    
    char *buffer;
    buffer = malloc(50);
    
    for(i=0; i<NUM_SENSORS; i++){
	char errstr[20] = "ERR", *errval, output[10];
	errval = malloc(3);
	modbus[i].begin(BAUD);
	if(status[i] == ON){
	    data = readReg(i, O2AVG_REG);
	    if(data < 0){ // Error
		// do something
	    }
	    else
		dtostrf(data, 2, 3, output);
	}
	else{	    
	    strcat(errstr, itoa(-1*err[i], errval, 10));
	    strcpy(output, errstr);
	    // Get status
	    if(k==100)
		handleSensor(i);
	}
	buf_ptr += snprintf(buffer+buf_ptr, 50-buf_ptr,
			    " %s ", output);
	free(errval);
    }
    Serial.println(buffer);
    Serial.flush();
    free(buffer);
    
    k+=1;
    k%=101;    
    // delay(100);
}


