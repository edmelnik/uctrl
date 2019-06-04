
#include <ModbusMaster.h>
#include <SoftwareSerial.h>

SoftwareSerial modbus(4, 5);
ModbusMaster node;

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
    Serial.print("Status: ");
    Serial.println(response);    
    if(response == 0){ // Sensor Idle
	// Turn sensor ON
	node.readHoldingRegisters(40006, 1);
    }
}

void loop(){
    int response;

    delay(500);
}

/* void loop() */
/* { */
/*     static uint32_t i; */
/*     uint8_t j, result; */
/*     uint16_t data[6]; */

/*     i++; */

/*     // set word 0 of TX buffer to least-significant word of counter (bits 15..0) */
/*     node.setTransmitBuffer(0, lowWord(i)); */

/*     // set word 1 of TX buffer to most-significant word of counter (bits 31..16) */
/*     node.setTransmitBuffer(1, highWord(i)); */

/*     // slave: write TX buffer to (2) 16-bit registers starting at register 0 */
/*     /\* result = node.writeMultipleRegisters(0, 2); *\/ */

/*     // slave: read (6) 16-bit registers starting at register 2 to RX buffer */
/*     result = node.readHoldingRegisters(0x9c46, 1); */
/*     Serial.println(result); */
/*     // do something with data if read is successful */
/*     if (result == node.ku8MBSuccess) */
/*     { */
/* 	for (j = 0; j < 6; j++) */
/* 	{ */
/* 	    data[j] = node.getResponseBuffer(j); */
/* 	    Serial.println(data[j]); */
/* 	} */
/*     } */
/* } */

/* #include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library */
/* #include <ArduinoModbus.h> */

/* int counter = 0; */

/* void setup() { */
/*   Serial.begin(9600); */
/*   while (!Serial); */

/*   Serial.println("Modbus RTU Client Kitchen Sink"); */

/*   // start the Modbus RTU client */
/*   if (!ModbusRTUClient.begin(9600)) { */
/*     Serial.println("Failed to start Modbus RTU Client!"); */
/*     while (1); */
/*   } */
/* } */

/* void loop() { */
/*   /\* writeCoilValues(); *\/ */

/*   /\* readCoilValues(); *\/ */

/*   /\* readDiscreteInputValues(); *\/ */

/*   /\* writeHoldingRegisterValues(); *\/ */

/*   readHoldingRegisterValues(); */

/*   /\* readInputRegisterValues(); *\/ */

/*   counter++; */

/*   delay(5000); */
/*   Serial.println(); */
/* } */

/* void readHoldingRegisterValues() { */
/*   Serial.print("Reading Input Register values ... "); */

/*   // read 10 Input Register values from (slave) id 42, address 0x00 */
/*   if (!ModbusRTUClient.holdingRegisterRead(3, 0x00)){ */
/*     Serial.print("failed! "); */
/*     Serial.println(ModbusRTUClient.lastError()); */
/*   } else { */
/*     Serial.println("success"); */

/*     while (ModbusRTUClient.available()) { */
/*       Serial.print(ModbusRTUClient.read()); */
/*       Serial.print(' '); */
/*     } */
/*     Serial.println(); */
/*   } */

/*   // Alternatively, to read a single Holding Register value use: */
/*   // ModbusRTUClient.holdingRegisterRead(...) */
/* } */
