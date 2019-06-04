
/* #include <ModbusMaster.h> */


/* ModbusMaster node; */

/* void setup(){ */
/*     Serial.begin(9600); // To USB output */
/*     Serial1.begin(9600); // To/from Oxygen sensors */
/*     node.begin(3, Serial1); */
/* } */

/* /\* void loop(){ *\/ */
/* /\*     int result = node.readHoldingRegisters(40006, 1); *\/ */
/* /\*     Serial.println(result); *\/ */
/* /\* } *\/ */

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

#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Modbus RTU Client Kitchen Sink");

  // start the Modbus RTU client
  if (!ModbusRTUClient.begin(9600)) {
    Serial.println("Failed to start Modbus RTU Client!");
    while (1);
  }
}

void loop() {
  /* writeCoilValues(); */

  /* readCoilValues(); */

  /* readDiscreteInputValues(); */

  /* writeHoldingRegisterValues(); */

  readHoldingRegisterValues();

  /* readInputRegisterValues(); */

  counter++;

  delay(5000);
  Serial.println();
}

void readHoldingRegisterValues() {
  Serial.print("Reading Input Register values ... ");

  // read 10 Input Register values from (slave) id 42, address 0x00
  if (!ModbusRTUClient.holdingRegisterRead(3, 0x00)){
    Serial.print("failed! ");
    Serial.println(ModbusRTUClient.lastError());
  } else {
    Serial.println("success");

    while (ModbusRTUClient.available()) {
      Serial.print(ModbusRTUClient.read());
      Serial.print(' ');
    }
    Serial.println();
  }

  // Alternatively, to read a single Holding Register value use:
  // ModbusRTUClient.holdingRegisterRead(...)
}
