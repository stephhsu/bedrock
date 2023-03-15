#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "SoftwareSerial.h"
#include <ESP8266HTTPClient.h>

const int RX_pin = 13; // for ESP8266 use 13  D7 on NodeMCU/wemos-d1-esp8266
const int TX_pin = 12; // for ESP8266 use 12  D6  on NodeMCU/wemos-d1-esp8266
SoftwareSerial soilSensorBoard(D7, D6); // RX, TX

// struct definitions
struct soil_data {
  uint32_t unix_timestamp;
  int moisture_value;
};

union inputFromBT {
  soil_data data;
  byte BTLine[6];
};

bool dataReceived = false; // whether data has been received from sensor board
bool dataPosted = false; // whether data has been posted to API

char soilSensorIds[2];
inputFromBT inputData[2];
byte BTData[6];
const char *ssid =  "ninanina";     // replace with your wifi ssid and wpa2 key
const char *pass =  "galileo2019";
String baseUrl = "http://3.133.160.82:4000";
bool startCommandReceived = false;
bool startCommandSentToMega = false;

void setup() {
  Serial.begin(9600);
  delay(10);
  // connect to wifi first
  Serial.println("Connecting to ");
  Serial.println(ssid); 
 
  WiFi.begin(ssid, pass); 
  connectToWifi();

  soilSensorBoard.begin(9600); // use previous rxPin, txPin and set 256 RX buffer. connect to soil sensor board
  Serial.println(F("ESP8266 Setup finished."));
}

void loop() {
  if (!startCommandReceived) {
    checkForStartCommand();
  } else if (!startCommandSentToMega) {
    Serial.println("Sending start char");
    soilSensorBoard.write('t');
    startCommandSentToMega = true;
    // while (!soilSensorBoard.available()) {
    //   Serial.println("No data from arduino yet");
    //   Serial.println(".");
    //   delay(5000);
    // }
    // for (byte i = 0; i < 2; i++) {
    //   receiveData(i);
    // }
    // Serial.println("delay before displaying");
    // delay(3000);
    // displayData();

    // //format data into json for sending to api
    // String json1 = formatSensorData(0);
    // String json2 = formatSensorData(1);
    // Serial.println("Checking wifi connection");
    // connectToWifi();
    // sendDataToAPI();
  }
  delay(5000);
}

void connectToWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void checkForStartCommand() {
  while (!startCommandReceived) {
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      String url = baseUrl + "/rover/cycle-status";
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, url.c_str());
      // http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        if (httpResponseCode == 204) {
          startCommandReceived = true;
        }
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    delay(10000);
  }
}

void receiveData(byte i) {
  if (soilSensorBoard.available() < 6) {
    return;
  }
  for (byte n = 0; n < 6; n++) {
    BTData[n] = soilSensorBoard.read();
  }
  
  for (byte n = 0; n < 6; n++) {
    inputData[i].BTLine[n] = BTData[n];
    Serial.print(n);
    Serial.print("<Receiving BTLine ... ");
    Serial.println(inputData[i].BTLine[n]);
  }
  soilSensorIds[i] = soilSensorBoard.read(); // last char of data received is the sensor id
  delay(7000);
}

void displayData() {
  for (byte i = 0; i < 2; i++) {
    Serial.print("<Soil data: ");
    Serial.print(inputData[i].data.unix_timestamp);
    Serial.print(" ");
    Serial.print(inputData[i].data.moisture_value);
    Serial.print(" ");
    Serial.println(soilSensorIds[i]);
  }
  dataReceived = true;
}

void sendDataToAPI() {
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      String url = baseUrl + "/collections";
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, url.c_str());
      http.addHeader("Content-Type", "application/json");
      String formattedData = formatJSON();
      Serial.println(formattedData);
      int httpResponseCode = http.POST(formattedData);
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        // Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
  else {
    Serial.println("WiFi Disconnected");
  }
  dataPosted = true;
}

String formatJSON() {
  String json = "{ \"data\": [";
  for (byte i = 0; i < 2; i++) {
    json = json + formatSensorData(i);
    if (i != 1) {
      json = json + String(",");
    } else {
      json = json + String("]}");
    }
  }
  return json;
}

String formatSensorData(byte i) {
  if (i == 1) {
    return String("{\"sensor_id\": \"a\",\"sensor_data\": [{\"timestamp\": \"1678507857\",\"value\": \"450\"}]}");
  } else {
    return String("{\"sensor_id\": \"b\",\"sensor_data\": [{\"timestamp\": \"1678509505\",\"value\": \"550\"}]}");
  }
}
