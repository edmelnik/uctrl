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
#define ONOFF_REG   40001
#define CLCTRL_REG  40004 // Calibration control
#define ADDR_REG    40006
#define ERR_CLR_REG 40002

// Starting PIN on the microcontroller
const static unsigned int START_PIN PROGMEM = 4;
const static unsigned int CHK_DELAY = 10;
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

ModbusMaster *node =malloc(sizeof(ModbusMaster)*4);

// Status < 0 means there's an active error code in err[] for the sensor
// Error == 0 means that there's sensor status value = valid
int status[NUM_SENSORS], cal[NUM_SENSORS];

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
    int res;
    status[i] = readReg(i, STATUS_REG);
    if(status[i] == IDLE || status[i] == STANDBY){
	res = writeReg(i, ONOFF_REG, 1);
	if(res < 0)
	    status[i] = res;
	else
	    status[i] = readReg(i, STATUS_REG);
    }
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
    for(i=0; i<NUM_SENSORS; i++){
	if(status[i] == IDLE || status[i] == STANDBY)
	    status[i] = writeReg(i, ONOFF_REG, 1);
    }

    // At this point ideally all sensors are turned on.
    // Sensors that have an active error code and not turned on
    //  should be handled appropriately in loop()
}

int k=0;

void loop(){
    int i, buf_ptr = 0, data;    
    char *buffer;
    
    buffer = malloc(50);
    
    for(i=0; i<NUM_SENSORS; i++){
	char errstr[10] = "ERR", *errval, output[10];
	char statstr[10] = "STS";
	errval = malloc(3);

	if(status[i] == ON){
	    data = readReg(i, O2AVG_REG);
	    if(data < 0){ // Error
		handleSensor(i);
	    }
	    else{
		dtostrf(data, 4, 0, output);
	    }
	}
	else if(status[i] >= 0){
	    strcat(statstr, itoa(status[i], errval, 10));
	    strcpy(output, statstr);
	    if(k==CHK_DELAY)
		handleSensor(i);
	}
	else{	    
	    strcat(errstr, itoa(status[i]*-1, errval, 10));
	    strcpy(output, errstr);
	    if(k==CHK_DELAY)
		handleSensor(i);
	}
	buf_ptr += snprintf(buffer+buf_ptr, 50-buf_ptr,
			    " %s ", output);
	free(errval);
	delay(4);
    }
    
    Serial.println(buffer);
    Serial.flush();
    free(buffer);
    
    k+=1;
    k%=11;
    // delay(100);
}


