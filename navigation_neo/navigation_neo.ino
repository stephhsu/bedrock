#include <Adafruit_HMC5883_U.h>
#include <Adafruit_Sensor.h>
#include <math.h>
//#include <NeoHWSerial.h>
#include <NMEAGPS.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include "waypoint.h"

// setup motor controllers (driving)
int motor1pin1 = 8;
int motor1pin2 = 7;

// gps navigation
static const uint32_t GPSBaud = 9600;
NMEAGPS gps;
gps_fix fix;
double dist_to_target, course_to_target;
int course_change_needed;

// setup waypoints
#define NUM_OF_WAYPOINTS 2
int current_waypoint = -1;
NeoGPS::Location_t waypoints[NUM_OF_WAYPOINTS] = {{404381311L, -38196229L}}; // add waypoints to this array
// ex:
//{
//              { 123456789L, -987654321 },
//              { 123456789L, -987654321 }
//};

// constants for speeds
#define STOP 0
#define NORMAL_SPEED 255
int speed = NORMAL_SPEED;

void setup() {
  Serial.begin(9600); // connect serial
  
  // motor drivers (drive & steering)
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(9, OUTPUT);

  // gps
  Serial1.begin(GPSBaud);

  // setup for going to first waypoint
  nextWaypoint();
}

void loop() {
  // process info from GPS
  while (gps.available(Serial1)){
    fix = gps.read();

    if (fix.valid.location) {
      processGPS();
    } else {
      Serial.println( F("Waiting for GPS fix...") );
    }

    // within 1 meter of destination
    if (dist_to_target < 0.001) {
      nextWaypoint();
    }

    move();
  }
}

/* update distance and course to target, course change */
void processGPS() {
  dist_to_target = fix.location.DistanceKm(waypoints[current_waypoint]);
  course_to_target = fix.location.BearingToDegrees(waypoints[current_waypoint]);
  course_change_needed = (int)(360 + course_to_target - fix.heading()) % 360;
  Serial.print("lat: "); Serial.println(fix.latitude());
  Serial.print("long: "); Serial.println(fix.longitude());
  Serial.print("heading: "); Serial.println(fix.heading());
  Serial.print("dist_to_target: "); Serial.println(dist_to_target);
  Serial.print("course_to_target: "); Serial.println(course_to_target);
  Serial.print("course_change_needed: "); Serial.println(course_change_needed);
}

void move() {  
  if ((course_change_needed >= 315) && (course_change_needed < 45)) {
    // forward
    Serial.println("forwards");
    analogWrite(9, speed); //ENA pin
    digitalWrite(motor1pin1, HIGH);
    digitalWrite(motor1pin2, LOW);
  } else if ((course_change_needed < 225) && (course_change_needed >= 135)) {
  // backwards
    Serial.println("backwards");
    analogWrite(9, speed); //ENA pin
    digitalWrite(motor1pin1, LOW);
    digitalWrite(motor1pin2, HIGH);
  } else {
    Serial.println("neither");
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor1pin1, LOW);
  }
}

void nextWaypoint() {
  current_waypoint++;

  if (current_waypoint >= NUM_OF_WAYPOINTS) {
    Serial.println("Last waypoint reached");
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor1pin1, LOW);
  }
}
