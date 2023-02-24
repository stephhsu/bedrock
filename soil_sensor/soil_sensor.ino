#include <EEPROM.h>
#include <RTClib.h>
#include "soil_data.h"

#define ADDR 0
#define COUNT_ADDR 2

outputToBt outputData;
byte btData[6];
char c = ' ';

RTC_DS1307 rtc;
int eeAddress;
int count;

int lastHour = -1;

void setup() {
  // open serial port, set the baud rate to 9600 bps
  Serial.begin(38400);  // might be 9600
  Serial.setTimeout(10);
  delay(1000);

  // HC-05 default serial speed for AT mode is 38400
  Serial1.begin(38400); 

  if (!rtc.begin()) {
   Serial.println("Couldn't find RTC");
   while (1);
  }
}

void loop() {
  DateTime now = rtc.now();
  if (now.hour() != lastHour) {
    lastHour = now.hour();
    if (now.hour() % 3 == 0) {
      collectAndStoreSoilData();
    }
  }

  // data incoming from bluetooth module
  if (Serial1.available()) {
    c = Serial1.read();
    if (c == 'h') {
      retrieveAndSendSoilData();
    }
  }
}

void collectAndStoreSoilData() {
  // getting address where we can start storing
  EEPROM.get(ADDR, eeAddress);
  if (isnan(eeAddress)) {
    // start storing data info at address 4
    eeAddress = 4;
  }

  // getting current count of collected samples
  EEPROM.get(COUNT_ADDR, count);
  if (isnan(count)) {
    count = 0;
  }
  
  int soilMoistureValue = analogRead(A0);
  
  // grabbing the current unix timestamp from rtc module
  DateTime now = rtc.now();
  uint32_t nowUnix = now.unixtime();
  
  soil_data collected_data = {nowUnix, soilMoistureValue};

  // store collected data in the EEPROM
  EEPROM.put(eeAddress+(count*sizeof(soil_data)), collected_data);

  // increase the count and store it in EEPROM
  count++;
  EEPROM.put(COUNT_ADDR, count);
}

void retrieveAndSendSoilData() {
  EEPROM.get(ADDR, eeAddress);
  EEPROM.get(COUNT_ADDR, count);

  // let rover know in advance the data count
  Serial1.write(count);

  for (int i=0; i<count; i++) {
    EEPROM.get(eeAddress+(i*sizeof(soil_data)), outputData.data);
    sendSoilData();
  }

  // reset the count back to 0
  EEPROM.put(COUNT_ADDR, 0);
}

void sendSoilData () {
  for (byte n = 0; n < 6; n++) {
     BTData[n] = outputData.BTLine[n];
   }
   
  for (byte n = 0; n < 6; n++) {
      Serial.print("Sending:   "); Serial.println(BTData[n]);
      Serial1.write(BTData[n]);
   }
}
