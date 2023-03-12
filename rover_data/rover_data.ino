#include <EEPROM.h>
#include "Timer.h"                     //http://github.com/JChristensen/Timer
#include "SoftwareSerial.h"
#include <SoftwareSerial.h>

// var for nodemcu connection
const int RX_pin = 10;
const int TX_pin = 9;
SoftwareSerial nodemcuSerial(RX_pin, TX_pin); // RX, TX

// var for BT data transfer
#define STATE  11 //hc05 state to d11
#define KEY    12 //hc-05 key(soldered) to d12
#define EN     13 //hc-05 EN to pin 13

#define ADDR 0
#define COUNT_ADDR 2
#define NUM_SENSORS 2

Timer t;
int dynamicEvent;

int eeAddress;
int orginalAddress;
int totalCount = 0;
char c = ' ';
boolean newData = false;
int count = -1;
int inc;
boolean countReceived = false;

// soil data
struct soil_data {
  uint32_t unix_timestamp;
  int moisture_value;
};

union outputToBT {
  soil_data data;
  byte BTLine[6];
};

outputToBT inputData;
byte BTData[6];

boolean SETTINGHC05MODE = false;
int KEY_MODE;
int currentFunctionStep = 0;
boolean shouldConnectToSoilSensor = true;

String ATResponse;

String bt_addrs[NUM_SENSORS] = {"98da,50,0112af", "98d3,31,403fb0"};
char sensor_ids[NUM_SENSORS] = {'a', 'b'};
int sensors_data_count[NUM_SENSORS] = {0 ,0};
int currentSensor = 0; // update when navigation code signals

boolean isStartCommandReceived = false;
boolean isDataReceived = false;
boolean isReadyToSendData = false;

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

  //Init nodemcu serial
  nodemcuSerial.begin(9600);
  while(!nodemcuSerial) {} // wait for nodemcu serial
  Serial.println("Nodemcu ready!");
  
  disconnectFromSoilSensor();

  Serial.println("Clearing EEPROM!");
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

void loop() {
  // bluetooth logic
  if (!isDataReceived) {
    if (shouldConnectToSoilSensor) {
      connectToSoilSensor(bt_addrs[currentSensor]);
      shouldConnectToSoilSensor = false;
    }
  
    // TODO: signal from navigation code
    // update the currentSensor index
    // connectToSoilSensor = true;
  
    // sends a signal to soil sensor
    char sig = 'm';
    Serial1.write(sig); // on the sensor side, it will know to send data
    // sensor should send the number of soil data in advance
    delay(1000);
    Serial.println("data requested");
    receiveSoilDataCount();
  
    if (countReceived) {
      inc = 0;
      while (inc < sensors_data_count[currentSensor]) {
        receiveSoilData();
      }
      countReceived = false;
      disconnectFromSoilSensor();
  
      shouldConnectToSoilSensor = true;
      currentSensor++;
    }

    if (currentSensor ==  NUM_SENSORS) {
       isDataReceived = true;
    }
  } else if (!isReadyToSendData) {
    for (byte i = 0; i < 2; i++) {
      sendDataToNodeMcu(i);        
    }
    isReadyToSendData = true;    
  }
}

void receiveSoilDataCount() {
   if (Serial1.available() < 2 && Serial1.available() > 0) {
    sensors_data_count[currentSensor] = Serial1.read();
    Serial.println(sensors_data_count[currentSensor]);
    countReceived = true;
   }   
}

void receiveSoilData() { 
   if (Serial1.available() < 6) {
     // error
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

  EEPROM.get(ADDR, eeAddress);
  if (isnan(eeAddress) || eeAddress == 0) {
    // start storing data info at address 4
    eeAddress = 4;
  }

  if (currentSensor == 0) {
    orginalAddress = eeAddress;
  }
  Serial.print("eeAddress: "); Serial.println(eeAddress);
   
//  EEPROM.put(eeAddress+(inc*sizeof(soil_data)), inputData.data);
  EEPROM.put(eeAddress, inputData.data);
  EEPROM.put(ADDR, eeAddress+sizeof(outputToBT));
  Serial.print("storing data at eeAddress: "); Serial.println(eeAddress+(inc*sizeof(soil_data)));
  inc++;
}

void connectToSoilSensor(String addr) {
  Serial.println("Attempting to connect to " + addr);
  //set HC-05 to AT mode
  KEY_MODE = HIGH;   
  SETTINGHC05MODE= true;
  setHc05Mode(); 
  while(SETTINGHC05MODE){t.update();}

  //send commands
  delay(1000);
  sendAT("AT+BIND=" + addr);
  sendAT("AT+LINK=" + addr); // linking the devices - will end up in communication mode

  KEY_MODE = LOW;   
  SETTINGHC05MODE= true;
  setHc05Mode(); 
  while(SETTINGHC05MODE){t.update();}
  
  delay(5000);
}

void disconnectFromSoilSensor() {
  delay(2000);
  //set HC-05 to AT mode
  KEY_MODE = HIGH;   
  SETTINGHC05MODE= true;
  setHc05Mode(); 
  while(SETTINGHC05MODE){t.update();}

  //send commands
  delay(1000);
  sendAT("AT+DISC");
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

void sendDataToNodeMcu(byte j) {
  for (int i = 0; i < sensors_data_count[j]; i++) {
    outputToBT toDisplay;
    if (j == 0) {
      eeAddress = orginalAddress;
    } else {
      EEPROM.get(ADDR, eeAddress);
      Serial.print("eeAddress: "); Serial.println(eeAddress);
    }
    
    Serial.print("getting data from eeAddress: "); Serial.println(eeAddress+(i*sizeof(outputToBT)));
    EEPROM.get(eeAddress+(i*sizeof(outputToBT)), toDisplay);

    for (byte n = 0; n < 6; n++) {
      BTData[n] = toDisplay.BTLine[n];
    }
     
    for (byte n = 0; n < 6; n++) {
      Serial.print("Sending:   "); Serial.println(BTData[n]);
      nodemcuSerial.write(BTData[n]);
    }
  }

  // updates the address to where the sent data left off
  EEPROM.put(ADDR, eeAddress+(sensors_data_count[j]*sizeof(outputToBT)));
  Serial.print("updating eeAddress to: "); Serial.println(eeAddress+(sensors_data_count[j]*sizeof(outputToBT)));
  
  nodemcuSerial.write(sensor_ids[j]);
  Serial.print("Done sending data for sensor: ");Serial.println(sensor_ids[j]);
  delay(5000);
}
