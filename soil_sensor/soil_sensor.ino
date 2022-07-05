#include <RTClib.h>
#include <soil_data.h>

RTC_DS1307 rtc;
int eeAddress;

void setup() {
  // open serial port, set the baud rate to 9600 bps
  Serial.begin(9600);

  if (! rtc.begin()) {
   Serial.println("Couldn't find RTC");
   while (1);
  }

  EEPROM.get(0, eeAddress);
  if (isnan(eeAddress)) {
    // start storing data info at address 2
    eeAddress = 2;
  }
}

void loop() {
  int soilMoistureValue = analogRead(A0);
  Serial.println(soilMoistureValue);

  // grabbing the current unix timestamp from rtc module
  DateTime now = rtc.now();
  uint32_t nowUnix = now.unixtime();

  soil_data collected_data = {nowUnix, soilMoistureValue};

  // store it in the EEPROM
  EEPROM.put(eeAddress, collected_data);
  eeAddress += sizeof(soil_data);
  EEPROM.put(0, eeAddress);

  // TODO: check if bluetooth is in range
  // TODO: send data the data over bluetooth
  // TODO: clear the data from EEPROM
  // TODO: research if it is possible to power off instead of delay
  delay(250);
}
