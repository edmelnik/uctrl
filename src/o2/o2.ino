
#include <ModbusMaster.h>


ModbusMaster node;

void setup(){
    Serial.begin(9600); // To USB output
    Serial1.begin(9600); // To/from Oxygen sensors
    node.begin(6, Serial1);
}

void loop(){
    int result = node.readHoldingRegisters(40006, 1);
    Serial.println(result);
}
