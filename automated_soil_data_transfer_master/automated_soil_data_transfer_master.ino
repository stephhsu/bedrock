#include "Timer.h"                     //http://github.com/JChristensen/Timer
#include <EEPROM.h>
#include "SerialComs.h"
#include "SoftwareSerial.h"
#include <SoftwareSerial.h>

// var for nodemcu connection
const int RX_pin = 10;
const int TX_pin = 11;
SoftwareSerial nodemcuSerial(RX_pin, TX_pin); // RX, TX

// var for BT data transfer
#define STATE  11 //hc05 state to d11
#define KEY    12 //hc-05 key(soldered) to d12
#define EN     13 //hc-05 EN to pin 13
#define ADDR 0
#define COUNT_ADDR 2

Timer t;

int dynamicEvent;

boolean SETTINGHC05MODE = false;
int KEY_MODE;
int currentFunctionStep = 0;

String ATResponse;

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

char sensor_ids[2] = {'a', 'b'};
int sensors_data_count[2] = {0 ,0};
String bt_addrs[2] = {"98da,50,0112af", "98d3,31,403fb0"};

boolean newData = false;
boolean askForData = true;
int count = -1;
int inc;
boolean countReceived;

int eeAddress;



void setup() {
  pinMode(STATE, INPUT);
  pinMode(KEY, OUTPUT);
  pinMode(EN, OUTPUT);

  //Begin Serial
  Serial.begin(38400);
  while(!Serial){} //wait for serial
  Serial.println("Serial ready!");

  //Init nodemcu serial
  nodemcuSerial.begin(9600);
  while(!nodemcuSerial) {} // wait for nodemcu serial
  Serial.println("Nodemcu ready!")

  //Begin BT
  Serial1.begin(38400);
  while(!Serial1){} //Wait for BT serial
  Serial.println("Bluetooth ready!");


  for (byte i = 0; i < 2; i++) {
    countReceived = false;
    //Start HC-05 in AT mode:
    KEY_MODE = HIGH;   
    SETTINGHC05MODE= true;
    setHc05Mode(); 
    while(SETTINGHC05MODE){t.update();}
  
    delay(1000);
    sendAT("AT+DISC");
    sendAT("AT+BIND="+bt_addrs[i]);
  
    // linking the devices - will end up in communication mode
    sendAT("AT+LINK="+bt_addrs[i]); 
  
    KEY_MODE = LOW;   
    SETTINGHC05MODE= true;
    setHc05Mode(); 
    while(SETTINGHC05MODE){t.update();}
  
    delay(5000);
    
    while (!countReceived){
      Serial.println("asking for count");
      t.update(); 
      requestData();
      delay(1000);
      receiveSoilDataCount();
    }
  
    Serial.print("count received:" ); Serial.println(count);
    sensors_data_count[i] = count;
  
    inc = 0;
    while (inc < sensors_data_count[i]) {
      receiveSoilData();
    }
  
    delay(2000);
  
    KEY_MODE =  HIGH;   
    SETTINGHC05MODE= true;
    setHc05Mode(); 
    while(SETTINGHC05MODE){t.update();}
  
    delay(1000);
    sendAT("AT+DISC");
  
    Serial.print("Done receiving data from sensor "); Serial.println(sensor_ids[i]);
  }


  // in the real system, it would wait for a signal from the nodemcu before sending
  for (byte i = 0; i < 2; i++) {
    delay(2000);
    sendDataToNodeMcu(i);
  }

  
  Serial.println("Done process");
}

void loop() {
  t.update(); 
  // main loop functions should be here
  // waitForRoverStart()
  // get data from BT
  // waitForSendDataToNodeMcu() // includes sending data
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

void requestData() {
      char c = 'm';
      Serial.println("asking for data");
      Serial1.write(c);
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
  if (isnan(eeAddress)) {
    // start storing data info at address 4
    eeAddress = 4;
  }
   
  EEPROM.put(eeAddress+(inc*sizeof(soil_data)), inputData.data);
  inc++;
}

void receiveSoilDataCount() {
  if (Serial1.available() < 2) {
    count = Serial1.read();
    countReceived = true;
  }
}

// functions for nodemcu
void waitForRoverStart() {
  
}

void waitForSendDataToNodeMcu() {
  while (!nodemcuSerial.available()) {} // do nothing until we get a command from nodemcu
  char command = nodemcuSerial.read();
  Serial.println("Node command received");
  if (command = 's') {
    for (byte i = 0; i < 2; i++) {
      sendSoilData(i);        
    }
  }
}

void sendDataToNodeMcu(byte j) {
  for (int i = 0; i < sensors_data_count[j]; i++) {
    outputToBT toDisplay;
    EEPROM.get(eeAddress+(i*sizeof(outputToBT)), toDisplay);

    for (byte n = 0; n < 6; n++) {
      BTData[n] = toDisplay.BTLine[n];
    }
     
    for (byte n = 0; n < 6; n++) {
      Serial.print("Sending:   "); Serial.println(BTData[n]);
      nodemcuSerial.write(BTData[n]);
    }
  }

  nodemcuSerial.write(sensor_ids[j]);
  Serial.print("Done sending data for sensor: ");Serial.println(sensor_ids[j]);
  delay(500);
}
