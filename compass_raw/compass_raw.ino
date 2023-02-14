#include <Wire.h>
#include <Adafruit_HMC5883_U.h>

// setup compass
Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345);
sensors_event_t compass_event;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  if (!compass.begin()){
    Serial.println(F("COMPASS ERROR"));
    while (1);
  }
  Serial.println("Magnetometer Uncalibrated (Units in Nanotesla)"); 
}

void loop()
{
  compass.getEvent(&compass_event);
  float Xm_print, Ym_print, Zm_print;
  
  Xm_print = compass_event.magnetic.x*(100000.0/1090.0); // Gain X [LSB/Gauss] for selected sensor input field range (1.3 in these case)
  Ym_print = compass_event.magnetic.y*(100000.0/1090.0); // Gain Y [LSB/Gauss] for selected sensor input field range
  Zm_print = compass_event.magnetic.z*(100000.0/1090.0 );  // Gain Z [LSB/Gauss] for selected sensor input field range
  
  Serial.print(Xm_print, 10); Serial.print(" "); Serial.print(Ym_print, 10); Serial.print(" "); Serial.println(Zm_print, 10);
  delay(125);
}
