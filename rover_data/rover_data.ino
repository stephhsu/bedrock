#include <RTClib.h>
#include <sensor_data.h>
#include "Timer.h"                     //http://github.com/JChristensen/Timer

#define ADDR 0
#define COUNT_ADDR 2

#define STATE  11 //hc05 state to d11
#define KEY    12 //hc-05 key(soldered) to d12
#define EN     13 //hc-05 EN to pin 13

Timer t;
int dynamicEvent;

int eeAddress;
int count = 0;
int totalCount = 0;
char c = ' ';
boolean newData = false;
boolean countReceived = false;

boolean SETTINGHC05MODE = false;
int KEY_MODE;
int currentFunctionStep = 0;
boolean connectToSoilSensor = true;

String ATResponse;

String addrs[2] = {"98da,50,0112af", "fill in later"};
int currentSensor = 0; // update when navigation code signals

void setup() {
  // set up for enabling AT mode
  pinMode(STATE, INPUT);
  pinMode(KEY, OUTPUT);
  pinMode(EN, OUTPUT);

  // begin serial
  Serial.begin(38400);
  while(!Serial){} //wait for serial
 
  // begin bt
  Serial1.begin(38400);
  while(!Serial1){} //Wait for BT serial
  Serial.println("Bluetooth ready!");
}

void loop() {
  // bluetooth logic
  if (connectToSoilSensor) {
    connectToSoilSensor(addrs[currentSensor]);
    connectToSoilSensor = false;
  }

  // TODO: signal from navigation code
  // update the currentSensor index
  // connectToSoilSensor = true;

  // sends a signal to soil sensor
  char signal = 'h';
  Serial1.write(signal); // on the sensor side, it will know to send data
  // sensor should send the number of soil data in advance
  receiveSoilDataCount();

  if (countReceived) {
    for (int i = 0; i<count; i++) {
      receiveSoilData();
      collectAndStoreSoilDataFromSensor();
    }
    countReceived = false;
    disconnectFromSoilSensor(addrs[currentSensor]);
  }

  // TODO @nina
  // wait for a signal from node mcu
  // once signal is received, send the collected data over
}

void collectAndStoreSoilDataFromSensor() {
  if (newData == false) {
     return;
  }
  
  EEPROM.get(0, eeAddress);
  if (isnan(eeAddress)) {
    // start storing data info at address 4
    eeAddress = 4;
  }
  
  EEPROM.get(ADDR, eeAddress);
  
  for (int i=0; i<count; i++) {
    // double check this logic
    EEPROM.put(eeAddress+(count*sizeof(soil_data)), inputData.data);
  }

  EEPROM.put(COUNT_ADDR, totalCount+count);
}

void receiveSoilDataCount() {
   if (Serial1.available() == 2) {
    count = Serial1.parseInt();
    countReceived = true;
   }   
}

void receiveSoilData() {
   if (Serial1.available() < 6) {
     // error: no soil data to receive
     return;
   }

   for (byte n = 0; n < 6; n++) {
      BTData[n] = Serial1.read();
      Serial.print("<Receiving BTData ... ");
      Serial.println(BTData[n]);
   }
   for (byte n = 0; n < 6; n++) {
     inputData.BTLine[n] = BTData[n];
     Serial.print("<Receiving BTLine ... ");
     Serial.println(inputData.BTLine[n]);
   }
   newData = true;
}

void connectToSoilSensor(String addr) {
  //set HC-05 to AT mode
  KEY_MODE = HIGH;   
  SETTINGHC05MODE= true;
  setHc05Mode(); 
  while(SETTINGHC05MODE){t.update();}

  //send commands
  delay(1000);
  sendAT("AT+BIND=" + addr);
  sendAT("AT+LINK=" + addr); // linking the devices - will end up in communication mode
}

void disconnectFromSoilSensor(String addr) {
  //set HC-05 to AT mode
  KEY_MODE = HIGH;   
  SETTINGHC05MODE= true;
  setHc05Mode(); 
  while(SETTINGHC05MODE){t.update();}

  //send commands
  delay(1000);
  sendAT("AT+DISC=" + addr);
}

void sendAT(String Command){
  Serial.print("Command:" ); Serial.println(Command);
  Serial1.println(Command);
  delay(2000);
    if(Serial1.available()>0){
      Serial.print("Response: ");
      ATResponse = Serial1.readStringUntil('OK\r\n ');
      Serial.println(ATResponse);
      Serial.println(" ");
    }
}

void setHc05Mode(){
  if (currentFunctionStep == 0) { //if we haven't turned EN low yet
    Serial.print("Setting HC-05 To ");
    if (KEY_MODE == HIGH) { Serial.println("AT mode"); }
    else if (KEY_MODE == LOW) { Serial.println("Communication mode"); }
    Serial.println("Debugging:");
    digitalWrite(EN, LOW); //pull low so we can change modes

    Serial.print("      "); Serial.println("EN LOW");
    currentFunctionStep++; //don't repeat this
    dynamicEvent = t.after(200,setHc05Mode); //let's give it 200ms before we continue
  } else if (currentFunctionStep == 1) {
    digitalWrite(KEY, KEY_MODE); //Change the key pin
    Serial.print("      "); Serial.print("KEY "); Serial.println(KEY_MODE);
    currentFunctionStep++; 
    dynamicEvent = t.after(200,setHc05Mode);
  } else if (currentFunctionStep == 2) {
    digitalWrite(EN, HIGH); //Stop changing modes
    Serial.print("      "); Serial.println("EN HIGH");
    currentFunctionStep++;
    if (KEY_MODE == HIGH){ //aka AT
      dynamicEvent = t.after(3000,setHc05Mode); //Let's wait 3 sec to change to AT
    } else if (KEY_MODE == LOW){ //aka comm
      dynamicEvent = t.after(5000,setHc05Mode); //Let's wait 5 sec to change to comm... bootup time is longer 
    }   
  } else if (currentFunctionStep == 3){ //If we've finished
    currentFunctionStep = 0; //reset progress
    SETTINGHC05MODE = false; //don't follow through again
    Serial.print("      "); Serial.println(">>READY");
  }
}
