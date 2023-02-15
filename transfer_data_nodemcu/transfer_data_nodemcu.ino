// use this with node_mcu_receive
// test out receiving a signal
// test out the data structs

#include <SoftwareSerial.h>

SoftwareSerial espSerial(5, 6);
char c = ' ';
struct soil_data {
  uint32_t unix_timestamp;
  int moisture_value;
};

union inputFromBT {
  soil_data data;
  byte BTLine[6];
};

inputFromBT inputData;
byte BTData[6];

void setup(){
  Serial.begin(9600);
  espSerial.begin(9600);
  delay(2000);
}

void loop(){
  if (espSerial.available()) {
    c = espSerial.read();
    if (c == 'm') {
      outputData.data = {45, 567};
      sendSoilData();
    }
  }

  delay(2000);
}

void sendSoilData () {
  for (byte n = 0; n < 6; n++) {
     BTData[n] = outputData.BTLine[n];
   }
   
  for (byte n = 0; n < 6; n++) {
      Serial.print("Sending:   "); Serial.println(BTData[n]);
      espSerial.write(BTData[n]);
   }
}
