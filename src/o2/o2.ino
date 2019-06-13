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
#define CALSTS_REG 30018
// Holding regs
#define ONOFF_REG   40001
#define CLCTRL_REG  40004 // Calibration control
#define ADDR_REG    40006
#define ERR_CLR_REG 40002

// Starting PIN on the microcontroller
const static unsigned int START_PIN PROGMEM = 4;
// Delay for periodic checks (status, calibration et cetera)
const static unsigned int CHK_DELAY = 30;

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

// SoftwareSerial modbus[NUM_SENSORS] = {
//     SoftwareSerial(START_PIN, START_PIN+1),
//     SoftwareSerial(START_PIN+2, START_PIN+3),
//     SoftwareSerial(START_PIN+4, START_PIN+5),
//     SoftwareSerial(START_PIN+6, START_PIN+7),
// };

SoftwareSerial modbus(4, 5);
// SoftwareSerial *modbus = malloc(sizeof(SoftwareSerial)*4);

// ModbusMaster node[NUM_SENSORS];

ModbusMaster *node = malloc(sizeof(ModbusMaster)*4);

/*
  Status < 0 means there's an active error code in err[] for the sensor
  Error == 0 means that there's sensor status value = valid

  cal_mark tracks the calibration demands from the interrupt pins,
  while cal tracks the actual calibration state of the boards
  cal_mark[i] == 1 means that calibration is process for sensor i
  this calibration could be DONE, PROG or IDLE.. cal[i] would hold
  that info
*/
int status[NUM_SENSORS], cal[NUM_SENSORS], cal_mark[NUM_SENSORS] = {0};

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
    status[i] = readReg(i, STATUS_REG); delay(1);
    cal[i] = readReg(i, CALSTS_REG); delay(1);

    if(cal_mark[i] == 1 && (cal[i] == CAL_IDLE || cal[i] == CAL_DONE)){ // Calibrate

	res = writeReg(i, CLCTRL_REG, 1); delay(1);
	if(res < 0)
	    status[i] = res;
	else{
	    cal[i] = readReg(i, CALSTS_REG); delay(1);
	    Serial.print("cal status: ");
	    Serial.println(cal[i]);
	}
    }
    else if(cal[i] == CAL_DONE){ // Reset
	res = writeReg(i, CLCTRL_REG, 2); delay(1);
	if(res < 0)
	    status[i] = res;
	else{
	    cal[i] = readReg(i, CALSTS_REG); delay(1);
	    cal_mark[i] = cal[i];
	    Serial.println("reset cal");
	}
    }
    else if((status[i] == IDLE || status[i] == STANDBY) && cal[i] != CAL_PROG){ // Turn ON
	res = writeReg(i, ONOFF_REG, 1); delay(1);
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
       
    errval = malloc(3);
    
    if(cal_out > 0){
	strcat(calstr, itoa(cal[sensor], errval, 10));
	strcpy(cal_out, calstr);
	retval = 1;
    }
    else if(status[sensor] == ON){
	data = readReg(sensor, O2AVG_REG);
	if(data < 0){ // Error
	    retval = -1;
	}
	else{
	    dtostrf(data, 4, 0, o2_str);
	    strcpy(output, o2_str);
	    retval = 1;
	}
    }
    else{
	if(status[sensor] >= 0){
	    strcat(statstr, itoa(status[sensor], errval, 10));
	    strcpy(output, statstr);
	    retval = -2;
	}
	else{	    
	    strcat(errstr, itoa(status[sensor]*-1, errval, 10));
	    strcpy(output, errstr);
	    retval = -3;
	}
    }
    free(errval);
    return retval;
}

void setup(){    
    int i, curr_pin, SST_addr;    
    Serial.begin(BAUD); // To USB output
    modbus.begin(BAUD);
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

int k=0;

void loop(){
    int i, buf_ptr = 0, data, retval, handle_flag[] = {0,0,0,0};    
    char *buffer, *output;    
    
    if(k == CHK_DELAY && digitalRead(12) == LOW){ // Calibration check (single cal for now)
	cal_mark[0] = 1;
	handle_flag[0] = 1;
    }

    buffer = malloc(50);
    for(i=0; i<NUM_SENSORS; i++){
	output = malloc(10);
	if(cal[i] == CAL_PROG){
	    retval = getVal(i, output, 1); // Calibration output
	    handle_flag[i] = 1;
	}
	else{
	    retval = getVal(i, output, 0); // Data output
	    if(retval < 0)
		handle_flag[i] = 1;
	}
	
	buf_ptr += snprintf(buffer+buf_ptr, 50-buf_ptr,
			    " %s ", output);
	free(output);
	delay(4);
    }	
    Serial.println(buffer);
    Serial.flush();
    free(buffer);
	
    for(i=0; k == CHK_DELAY && i<NUM_SENSORS; i++){
	if(handle_flag[i] == 1){
	    handleSensor(i);
	    delay(10);
	}
    }
    
    k+=1;
    k%=(CHK_DELAY+1);    
}


