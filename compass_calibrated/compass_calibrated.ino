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
Serial.println("Magnetometer Calibrated (Units in Nanotesla)"); 
}

void loop()
{
compass.getEvent(&compass_event);
float Xm_off, Ym_off, Zm_off, Xm_cal, Ym_cal, Zm_cal;

Xm_off = compass_event.magnetic.x*(100000.0/1090.0) + 173.709879; //X-axis combined bias (Non calibrated data - bias)
Ym_off = compass_event.magnetic.y*(100000.0/1090.0) + 1683.932284; //Y-axis combined bias (Default: substracting bias)
Zm_off = compass_event.magnetic.z*(100000.0/1090.0 ) - 859.845243; //Z-axis combined bias

Xm_cal =  0.37959*Xm_off + 0.004076*Ym_off + 0.005455*Zm_off; //X-axis correction for combined scale factors (Default: positive factors)
Ym_cal =  0.004076*Xm_off + 0.042996*Ym_off + 0.007544*Zm_off; //Y-axis correction for combined scale factors
Zm_cal =  0.005455*Xm_off + 0.007544*Ym_off + 0.042674*Zm_off; //Z-axis correction for combined scale factors

Serial.print(Xm_cal, 10); Serial.print(" "); Serial.print(Ym_cal, 10); Serial.print(" "); Serial.println(Zm_cal, 10);
delay(125);
}
