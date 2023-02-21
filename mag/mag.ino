/***************************************************************************
  This is a library example for the HMC5883 magnentometer/compass
 
  Designed specifically to work with the Adafruit HMC5883 Breakout
  http://www.adafruit.com/products/1746
 
  *** You will also need to install the Adafruit_Sensor library! ***
 
  These displays use I2C to communicate, 2 pins are required to interface.
 
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
 
  Written by Kevin Townsend for Adafruit Industries with some heading example from
  Love Electronics (loveelectronics.co.uk)
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the version 3 GNU General Public License as
 published by the Free Software Foundation.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 ***************************************************************************/
 
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
 
/* Assign a unique ID to this sensor at the same time */
Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345);
sensors_event_t compass_event;
 
void displaySensorDetails(void)
{
  sensor_t sensor;
  compass.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}
 
void setup(void) 
{
  Serial.begin(9600);
  Serial.println("HMC5883 Magnetometer Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!compass.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displaySensorDetails();
}
 
void loop(void) 
{
  /* Get a new sensor event */ 
  compass.getEvent(&compass_event);

  float Xm_off, Ym_off, Zm_off, Xm_cal, Ym_cal, Zm_cal;
  Xm_off = compass_event.magnetic.x*(100000.0/1090.0) - 757.046265; //X-axis combined bias (Non calibrated data - bias)
  Ym_off = compass_event.magnetic.y*(100000.0/1090.0) + 2194.651668; //Y-axis combined bias (Default: substracting bias)
  Zm_off = compass_event.magnetic.z*(100000.0/1090.0 ) - 2441.668186; //Z-axis combined bias
  
  Xm_cal =  0.037155*Xm_off - 0.002887*Ym_off + 0.000007*Zm_off; //X-axis correction for combined scale factors (Default: positive factors)
  Ym_cal =  -0.002887*Xm_off + 0.041444*Ym_off + 0.002247*Zm_off; //Y-axis correction for combined scale factors
  Zm_cal =  0.000007*Xm_off + 0.002247*Ym_off + 0.036391*Zm_off; //Z-axis correction for combined scale factors
 
//  Serial.print("X: "); Serial.print(Xm_cal); Serial.print("  ");
//  Serial.print("Y: "); Serial.print(Ym_cal); Serial.print("  ");
//  Serial.print("Z: "); Serial.print(Zm_cal); Serial.print("  ");Serial.println("uT");
 
  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(Ym_cal, Xm_cal);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  float declinationAngle = 0.15708; // Waterloo, ON
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = (heading) * 180/M_PI; 
  headingDegrees += 70;
  // Correct for when signs are reversed.
  if(headingDegrees < 0)
    headingDegrees += 360;
    
  // Check for wrap due to addition of declination.
  if(headingDegrees > 360)
    headingDegrees -= 360;
  
  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);
  
  delay(1000);
}
