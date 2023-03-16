#include <NMEAGPS.h>
#include <SoftwareSerial.h>

// setup motor controllers (driving)
// motor a connections
int enA = 9;
int motor1pin1 = 8;
int motor1pin2 = 7;
// motor b connections
int enB = 3;
int motor2pin1 = 5;
int motor2pin2 = 4;
// motor c connections
int enC = 6;
int motor3pin1 = 2;
int motor3pin2 = 13;
// motor d connections
int enD = 10;
int motor4pin1 = 12;
int motor4pin2 = 11;

static const int RXPin = 4, TXPin = 3;
SoftwareSerial ss(RXPin, TXPin);

int delays[9] = {0,0,0,0,0,0,0,0,0}; // fill in later
String actions[9] = {"forwards", "wait", "forwards", "wait", "forwards", "left", "forwards", "left", "forwards"}; // fill in later
int inc = 0;


void setup() {
  Serial.begin(9600); // connect serial
  
  // set motor drivers control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(enC, OUTPUT);
  pinMode(enD, OUTPUT);
  pinMode(motor3pin1, OUTPUT);
  pinMode(motor3pin2, OUTPUT);
  pinMode(motor4pin1, OUTPUT);
  pinMode(motor4pin2, OUTPUT);
  
  // initial state: turn off motors
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  digitalWrite(motor3pin1, LOW);
  digitalWrite(motor3pin2, LOW);
  digitalWrite(motor4pin1, LOW);
  digitalWrite(motor4pin2, LOW);

  // communication with data board
  Serial2.begin(38400);
}

void loop() {
  if (actions[inc] == "forwards") {
    Serial.println("forward");
    analogWrite(enA, 200);
    analogWrite(enB, 200);
    analogWrite(enC, 200);
    analogWrite(enD, 200);
    digitalWrite(motor1pin1, HIGH);
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
    digitalWrite(motor3pin1, HIGH);
    digitalWrite(motor3pin2, LOW);
    digitalWrite(motor4pin1, LOW);
    digitalWrite(motor4pin2, HIGH);
  } else if (actions[inc] == "wait") {
    stop_rover();
    Serial2.write('y');
    
    // wait for a signal from the data board before moving on
    while (!Serial2.available()) {
      Serial.println("No signal for data board yet");
      delay(5000);
    }
    
  } else if (actions[inc] == "left") {
    Serial.println("left");
    analogWrite(enA, 255);
    analogWrite(enB, 80);
    analogWrite(enC, 80);
    analogWrite(enD, 255);
    digitalWrite(motor1pin1, HIGH);
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
    digitalWrite(motor3pin1, HIGH);
    digitalWrite(motor3pin2, LOW);
    digitalWrite(motor4pin1, LOW);
    digitalWrite(motor4pin2, HIGH);
    
  } else {
    stop_rover();
  }

  delay(delays[inc]);
  inc++;
}

void stop_rover() {
  digitalWrite(motor1pin1, LOW); 
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  digitalWrite(motor3pin1, LOW); 
  digitalWrite(motor3pin2, LOW);
  digitalWrite(motor4pin1, LOW);
  digitalWrite(motor4pin2, LOW);
}
