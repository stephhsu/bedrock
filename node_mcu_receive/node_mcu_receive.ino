// use this with transfer_data_nodemcu
// test out sending a signal
// test out receiving data structs
#include <SoftwareSerial.h>

SoftwareSerial nodemcu(D6, D5); // Rx, Tx

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

boolean newData = false;
boolean askForData = true;

void setup() {
  Serial.begin(9600);
  nodemcu.begin(9600);
  while (!Serial) continue;
}

void loop() {
  requestData();
  receiveData();
  displayData();
  delay(2000);
}

void requestData() {
   if (askForData) {
      int c = 'm';
      Serial.println("asking for data");
      nodemcu.write(c);
   }
}

void receiveData() {
   if (nodemcu.available() < 6) {
     // error
     Serial.println("no data");
     return;
   }
   for (byte n = 0; n < 6; n++) {
      BTData[n] = nodemcu.read();
      Serial.print("<Receiving BTData ... ");
      Serial.println(BTData[n]);
   }
   
   for (byte n = 0; n < 6; n++) {
     inputData.BTLine[n] = BTData[n];
     Serial.print("<Receiving BTLine ... ");
     Serial.println(inputData.BTLine[n]);
   }
   newData = true;
}

void displayData() {
  if (newData == false) {
     return;
  }
  askForData = false;
  Serial.print("<Soil data: ");
  Serial.print(inputData.data.unix_timestamp);
  Serial.print(" ");
  Serial.println(inputData.data.moisture_value);
  newData = false;
  askForData = true;
}
