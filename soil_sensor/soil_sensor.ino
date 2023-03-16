#include <EEPROM.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

#define ADDR 0
#define COUNT_ADDR 2

SoftwareSerial BTSerial(2, 3); // RX | TX

struct soil_data {
  uint32_t unix_timestamp;
  int moisture_value;
};

union outputToBt {
  soil_data data;
  byte BTLine[6];
};

outputToBt outputData;
byte BTData[6];
char c = ' ';

RTC_DS1307 rtc;
int eeAddress;
int count;

int inc = 0;
//int lastHour = -1;

void setup() {
  // open serial port, set the baud rate to 9600 bps
  Serial.begin(38400);  // might be 9600
  Serial.println("SLAVE MODULE 1");

  BTSerial.begin(38400);

  if (!rtc.begin()) {
   Serial.println("Couldn't find RTC");
   while (1);
  }

  Serial.println("clearing EEPROM");
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  
  collectAndStoreSoilData();
}

void loop() {
//  DateTime now = rtc.now();
//  if (now.hour() != lastHour) {
//    lastHour = now.hour();
//    if (now.hour() % 1 == 0) { // set to every 3 hours. change to 1 hour for demo
//      collectAndStoreSoilData();
//    }
//  }

  // data incoming from bluetooth module
  if (BTSerial.available()) {  
    c = BTSerial.read();
    if (c == 'm') {

    EEPROM.get(ADDR, eeAddress);
    EEPROM.get(COUNT_ADDR, count);

    // let rover know in advance the data count
    BTSerial.write(count);
    delay(5000);

    for (int i=0; i<count; i++) {
      EEPROM.get(eeAddress+(i*sizeof(soil_data)), outputData.data);
      sendSoilData();
      delay(2000);
    }

    //EEPROM.put(COUNT_ADDR, 0);
    } else {
      Serial.print("received");
      Serial.println(c);
    }
  }  
}

void collectAndStoreSoilData() {
  // getting address where we can start storing
  EEPROM.get(ADDR, eeAddress);
  if (isnan(eeAddress) || eeAddress == 0 ) {
    // start storing data info at address 4
    eeAddress = 4;
  }

  // getting current count of collected samples
  EEPROM.get(COUNT_ADDR, count);
  Serial.print("count: "); Serial.println(count);
  if (isnan(count)) {
    count = 0;
  }
  
  int soilMoistureValue = analogRead(A0);
//  DateTime now = rtc.now(); // grabbing the current unix timestamp from rtc module
//  uint32_t nowUnix = now.unixtime();
  
  soil_data collected_data = {1678992207, soilMoistureValue};
  inc++;

  // store collected data in the EEPROM
  EEPROM.put(eeAddress+(count*sizeof(soil_data)), collected_data);

  // increase the count and store it in EEPROM
  count++;
  EEPROM.put(COUNT_ADDR, count);
}

void sendSoilData () {
  for (byte n = 0; n < 6; n++) {
     BTData[n] = outputData.BTLine[n];
   }
   
  for (byte n = 0; n < 6; n++) {
      Serial.print("Sending:   "); Serial.println(BTData[n]);
      BTSerial.write(BTData[n]);
   }
}
