#include "SerialComs.h"
#include "SoftwareSerial.h"
#include <SoftwareSerial.h>

const int RX_pin = 10;
const int TX_pin = 9;

SoftwareSerial nodemcuSerial(RX_pin, TX_pin); // RX, TX

struct soil_data {
  uint32_t unix_timestamp;
  int moisture_value;
};

union inputFromBT {
  soil_data data;
  byte BTLine[6];
};

char sensor_ids[2] = {'a', 'b'};
inputFromBT inputData[2]= {{1676487106, 567}, {1676490000, 500}};
byte BTData[6];

void setup() {
  Serial.begin(115200);
  for (int i = 10; i > 0; i--) {
    Serial.print(' '); Serial.print(i);
    delay(500);
  }
  Serial.println();
  Serial.println("SoilSensorBoard");

  nodemcuSerial.begin(9600);   // Initialize the serial port to nodemcu

  Serial.println(F("Uno Setup finished."));
}

bool dataSent = false;

void loop() {
  Serial.println("checking for command");
  if (nodemcuSerial.available()) {
    char command = nodemcuSerial.read();
    Serial.println(command);
    Serial.println("Node command received");
    if (command = 's') {
      for (byte i = 0; i < 2; i++) {
        sendSoilData(i);        
      }
    }
  }
  delay(5000);
}

void sendSoilData (byte i) {
  Serial.print("sending soil data for sensor: ");
  Serial.println(i);
  for (byte n = 0; n < 6; n++) {
    BTData[n] = inputData[i].BTLine[n];
  }
   
  for (byte n = 0; n < 6; n++) {
    Serial.print("Sending:   "); Serial.println(BTData[n]);
    nodemcuSerial.write(BTData[n]);
  }

  nodemcuSerial.write(sensor_ids[i]);
  Serial.println("Done sending data for one sensor");
  delay(500);
}
