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
String baseUrl = "http://18.217.62.57:4000";

void setup() {
  Serial.begin(9600);
  delay(10);
  // connect to wifi first
  Serial.println("Connecting to ");
  Serial.println(ssid); 
 
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
  Serial.println("");
  Serial.println("WiFi connected");

  soilSensorBoard.begin(9600); // use previous rxPin, txPin and set 256 RX buffer. connect to soil sensor board
  Serial.println(F("ESP8266 Setup finished."));
}

void loop() {
  if (dataReceived == false) {
    soilSensorBoard.write('s');
    while (!soilSensorBoard.available()) {
      Serial.println("No data from arduino yet");
      delay(5000);
    }
    for (byte i = 0; i < 2; i++) {
      receiveData(i);
    }
    displayData();
    String json1 = formatSensorData(0);
    String json2 = formatSensorData(1);
    Serial.println(".");
    Serial.println(".");
    Serial.println(".");
    // sendDataToAPI();
  }
  delay(5000);
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
      Serial.print("<Receiving BTLine ... ");
      Serial.println(inputData[i].BTLine[n]);
    }
    soilSensorIds[i] = soilSensorBoard.read(); // last char of data received is the sensor id
    delay(2000);
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
        Serial.println(payload);
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
  String result = String("{\"sensor_id\": \" ") + soilSensorIds[i] + String("\",\"sensor_data\": [{\"timestamp\": ") + inputData[i].data.unix_timestamp + String(",\"value\": ") + inputData[i].data.moisture_value + String("}]}");
  return result;
}
