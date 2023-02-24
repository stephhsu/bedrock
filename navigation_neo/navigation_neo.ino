#include <Adafruit_HMC5883_U.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include <NMEAGPS.h>
#include <SoftwareSerial.h>
#include "waypoint.h"

// setup motor controllers (driving)
// motor a connections
int enA = 9;
int motor1pin1 = 8;
int motor1pin2 = 7;
// motor b connections
int enB = 3;
int motor2pin1 = 5;
int motor2pin2 = 4;

// compass navigation
Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345);
sensors_event_t compass_event;
int heading_error;                // signed (+/-) difference between target and current
#define HEADING_TOLERANCE 5       // error tolerance in degrees

// gps navigation
static const uint32_t GPSBaud = 9600;
NMEAGPS gps;
gps_fix fix;
double dist_to_target, course_to_target;
int course_change_needed;

// waypoints
#define NUM_OF_WAYPOINTS 1
int current_waypoint = -1;
NeoGPS::Location_t waypoints[NUM_OF_WAYPOINTS] = {{434725128L, -805448082L}}; // add waypoints to this array - maybe this should be a decimal instead?

// constants for speeds
#define STOP 0
#define NORMAL_SPEED 255
int speed = NORMAL_SPEED;

static const int RXPin = 4, TXPin = 3;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Serial.begin(9600); // connect serial
  
  // set motor drivers control pins to outputs
	pinMode(enA, OUTPUT);
	pinMode(enB, OUTPUT);
	pinMode(motor1pin1, OUTPUT);
	pinMode(motor1pin2, OUTPUT);
	pinMode(motor2pin1, OUTPUT);
	pinMode(motor2pin2, OUTPUT);
	
	// initial state: turn off motors
	digitalWrite(motor1pin1, LOW);
	digitalWrite(motor1pin2, LOW);
	digitalWrite(motor2pin1, LOW);
	digitalWrite(motor2pin2, LOW);

  // compass
  if(!compass.begin()) {
    Serial.println("COMPASS ERROR");
    while(1);
  }

  // gps
  Serial1.begin(GPSBaud); // this could be serial 1 instead of ss

  // setup for going to first waypoint
  nextWaypoint();
}

void loop() {
  // process info from GPS
  while (gps.available(Serial1)){ // this could be serial 1 instead of ss
    fix = gps.read();

    if (fix.valid.location) {
      processGPSAndCompass();
    } else {
      Serial.println( F("Waiting for GPS fix...") );
    }

    // within 1 meter of destination
    if (dist_to_target < 0.001) {
      nextWaypoint();
      processGPSAndCompass();
    }

    move();
  }
}

/* update distance and course to target, course change */
void processGPSAndCompass() {
  // using the neo library's functions
  dist_to_target = fix.location.DistanceKm(waypoints[current_waypoint]);
  course_to_target = fix.location.BearingToDegrees(waypoints[current_waypoint]);
  int current_heading = readCompass();
  course_change_needed = (int)(360 + course_to_target - current_heading) % 360;

  // to help with debugging
  Serial.print("lat: "); Serial.println(fix.latitude());
  Serial.print("long: "); Serial.println(fix.longitude());
  Serial.print("heading: "); Serial.println(fix.heading());
  Serial.print("dist_to_target: "); Serial.println(dist_to_target);
  Serial.print("course_to_target: "); Serial.println(course_to_target);
  Serial.print("current heading: "); Serial.println(current_heading);
  Serial.print("course_change_needed: "); Serial.println(course_change_needed);

  delay(5000);
}

/* returns the heading from the compass (rover's heading) */
int readCompass() {
  compass.getEvent(&compass_event);

  // compass calibration
  float Xm_off, Ym_off, Zm_off, Xm_cal, Ym_cal, Zm_cal;
  Xm_off = compass_event.magnetic.x*(100000.0/1090.0) - 757.046265; //X-axis combined bias (Non calibrated data - bias)
  Ym_off = compass_event.magnetic.y*(100000.0/1090.0) + 2194.651668; //Y-axis combined bias (Default: substracting bias)
  Zm_off = compass_event.magnetic.z*(100000.0/1090.0 ) - 2441.668186; //Z-axis combined bias
  
  Xm_cal =  0.037155*Xm_off - 0.002887*Ym_off + 0.000007*Zm_off; //X-axis correction for combined scale factors (Default: positive factors)
  Ym_cal =  -0.002887*Xm_off + 0.041444*Ym_off + 0.002247*Zm_off; //Y-axis correction for combined scale factors
  Zm_cal =  0.000007*Xm_off + 0.002247*Ym_off + 0.036391*Zm_off; //Z-axis correction for combined scale factors
  
  // heading calculation
  float heading_rads = atan2(Ym_cal, Xm_cal);

  // WATERLOO: Magnetic Declination: -9° 29' NEGATIVE (WEST)
  // 1 degreee = 0.0174532925 radians
  #define DECLINATION 0.157
  heading_rads += DECLINATION;

  // correct for when signs are reversed.
  if (heading_rads < 0)
    heading_rads += 2*PI;
    
  // check for wrap due to addition of declination.
  if (heading_rads > 2*PI)
    heading_rads -= 2*PI;
   
  // convert radians to degrees for readability.
  float heading_degs = (heading_rads) * 180/M_PI; 
  heading_degs -= 20; // compass random offset - change if needed

  if (heading_degs < 0)
    heading_degs += 360;
    
  if (heading_degs > 360)
    heading_degs -= 360;

  return static_cast<int>(heading_degs);
}

void move() {  
  // this code only accounts for going staight and turning and going directly backwards
  if ((course_change_needed >= 345) && (course_change_needed < 15)) {
    // forward
    Serial.println("forward");
    analogWrite(enA, 100);
	  analogWrite(enB, 100);
    digitalWrite(motor1pin1, LOW);
    digitalWrite(motor1pin2, HIGH);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, HIGH);
  } else if ((course_change_needed < 165) && (course_change_needed >= 15)) { 
    // turn right
    Serial.println("right");
    analogWrite(enA, 255); // might need to invert
	  analogWrite(enB, 80);
    digitalWrite(motor1pin1, LOW);
    digitalWrite(motor1pin2, HIGH);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, HIGH);
  } else if ((course_change_needed < 345) && (course_change_needed >= 195)) {
    // turn left
    Serial.println("left");
    analogWrite(enA, 80); // might need to invert
	  analogWrite(enB, 255);
    digitalWrite(motor1pin1, LOW);
    digitalWrite(motor1pin2, HIGH);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, HIGH);
  } else if ((course_change_needed < 195) && (course_change_needed >= 165)) {
    // backwards
    Serial.println("backward");
    analogWrite(enA, 100);
	  analogWrite(enB, 100);
    digitalWrite(motor1pin1, HIGH);
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
  }
}

// makes the next waypoint the current waypoint 
void nextWaypoint() {
  current_waypoint++;

  if (current_waypoint >= NUM_OF_WAYPOINTS) {
    Serial.println("Last waypoint reached");
    // turn off motors
    digitalWrite(motor1pin1, LOW); 
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, LOW);
  }
}
