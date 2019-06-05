/*
TODO Better error checking and overall control flow
  - Periodically monitor status and check expected O2 values
  - O2 values could be modelled to make intelligent calibration decisions
TODO Read multiple registers and parse the response buffer
  - Select relevent values (see comment above func loop)
TODO Format the output values in a similar manner to how the pressure is being formatted
  - Might be possible to use the output function as a header file common to both
TODO calibration routine
TODO When to use STANDBY mode on sensors?
TODO Combine o2 and pressure into a single grand PoC

Will the ~ PWM pins with this? Should any strange and unexpected errors occur in the near future, blame this
*/

/*

Response codes See 
0  Success
-1 invalid response slave ID exception
-2 invalid response function exception
-3 response timed out exception
-4 invalid response CRC exception
4-20ma.io/ModbusMaster/group__constant.html#gaf69c1c360b84adc5bf1c7bbe071f1970

 */
#include <ModbusMaster.h>
#include <SoftwareSerial.h>

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

// System status
const int IDLE     = 0;
const int STARTUP  = 1;
const int ON       = 2;
const int SHUTDOWN = 3;
const int STANDBY  = 4;

SoftwareSerial modbus[NUM_SENSORS] = {
    SoftwareSerial(4, 5),
    SoftwareSerial(6, 7),
    SoftwareSerial(8, 9),
    SoftwareSerial(10, 11)
};

ModbusMaster node[NUM_SENSORS];

void printStatus(int status){
    Serial.print("Status: ");
    Serial.println(status);
}

int getStatus(int sensor){
    int res, s;
    res = node[sensor].readInputRegisters(STATUS_REG, 1);
    if(res == 0)
	s = node[sensor].getResponseBuffer(0);
    else
	s = 0-(res % 223); // get ERR in [-1, -4]
    return s;
}

// If this func is taking an int parameter for value, can it be used for calibration?
int writeReg(int sensor, int reg, int value){
    int res;
    res = node[sensor].writeSingleRegister(reg, value);
    return res;
}

void setup(){
    int i;
    // These should be global
    int status[NUM_SENSORS], response[NUM_SENSORS], err[NUM_SENSORS];
    
    Serial.begin(BAUD); // To USB output    
    // Actual sensor addresses are indexed by 1, but
    // once sensor addresses are configured in node[], they should indexed by 0
    for(i=0; i<NUM_SENSORS; i++){
	modbus[i].begin(BAUD);
	node[i].begin(i+1, modbus[i]);
    }
    
    // get status
    for(i=0; i<NUM_SENSORS; i++)
	status[i] = getStatus(i);
    
    // If sensor is idle, turn it on
    for(i=0; i<NUM_SENSORS; i++){
	if(status[i] == IDLE)
	    err[i] = writeReg(i, ONOFF_REG, 1);
    }
    node.readInputRegisters(30004, 1);
    response = node.getResponseBuffer(0);
    printStatus(response);
    
    if(response == 0){ // Sensor Idle
    	// Turn sensor ON
    	node.writeSingleRegister(40001, 1);
    }
    /* 	// Wait while status changes to active (code 2) */
    /* 	while(response < 2){ */
    /* 	    node.readInputRegisters(30004, 1); */
    /* 	    response = node.getResponseBuffer(0); */
    /* 	} */
    /* } */
}

// Output format
// STATUS CAL O2%avg O2%raw ERR Heater_voltage ppO2real Pressure PSTemp
// 30004 30018 30001 30002  30005 30006        30014    30016    30017
void loop(){
    int response=0;
    float data;
    if(response<2){
	node.readInputRegisters(30004, 1);
	response = node.getResponseBuffer(0);
	printStatus(response);
    }
    while(1){
	node.readInputRegisters(30001, 1);
	data = node.getResponseBuffer(0);
	Serial.print("O2%: ");
	Serial.println(data, 5);
	delay(500);
    }
}
