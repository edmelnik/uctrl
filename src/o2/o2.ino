/*
TODO Better error checking and overall control flow
  - Periodically monitor status and check expected O2 values
  - O2 values could be modelled to make intelligent calibration decisions
TODO Read multiple registers and parse the response buffer
  - Select relevent values (see comment above func loop)
TODO Format the output values in a similar manner to how the pressure is being formatted
  - Might be possible to use the output function as a header file common to both
TODO calibration routine
TODO Combine o2 and pressure into a single grand PoC

*/
#include <ModbusMaster.h>
#include <SoftwareSerial.h>

SoftwareSerial modbus(4, 5);
ModbusMaster node;

void printStatus(int status){
    Serial.print("Status: ");
    Serial.println(status);
}

void setup(){
    Serial.begin(9600); // To USB output
    modbus.begin(9600); // To/from Oxygen sensors
    node.begin(3, modbus);
    int response;
    // Get address
    node.readHoldingRegisters(40006, 1);
    response = node.getResponseBuffer(0);
    Serial.print("Sensor address: ");
    Serial.println(response);
    // get status
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
