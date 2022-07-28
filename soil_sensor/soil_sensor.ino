#include <EEPROM.h>
#include <RTClib.h>
#include "soil_data.h"

/* Phase 2 TODO:
 * - research if it is possible to power off
 */

#define ADDR 0
#define COUNT_ADDR 2

RTC_DS1307 rtc;
int eeAddress;
int count;

int lastHour = -1;

void setup() {
  // open serial port, set the baud rate to 9600 bps
  Serial.begin(9600);
  delay(1000);

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

  
  
  // TODO: check if bluetooth is in range
  // TODO: call retrieveAndSendSoilData()
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
  Serial.println("RETRIEVED DATA");

  for (int i=0; i<count; i++) {
    soil_data retrieved_data;
    EEPROM.get(eeAddress+(i*sizeof(soil_data)), retrieved_data);
    // TODO: send via serial over bluetooth
  }

  // reset the count back to 0
  EEPROM.put(COUNT_ADDR, 0);
}
