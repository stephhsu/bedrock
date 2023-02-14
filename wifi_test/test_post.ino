#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "ninanina";
const char* password = "galileo2019";

//Your Domain name with URL path or IP address with path
String serverName = "http://3.12.149.126:4000";

unsigned long timerDelay = 5000;
bool request = false;

void setup() {
  Serial.begin(9600);
       delay(10);
       Serial.println("Connecting to ");
       Serial.println(ssid); 
 
       WiFi.begin(ssid, password); 
       while (WiFi.status() != WL_CONNECTED) 
          {
            delay(500);
            Serial.print(".");
          }
      Serial.println("");
      Serial.println("WiFi connected"); 
      Serial.println("NodeMCU IP address:");
      Serial.print(WiFi.localIP());
}

void loop() {
  delay(3000);
  if (request == false) {
    getUsers();
  } else {    
  }
}

void getUsers() {
      if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      String serverPath = serverName + "/collections";
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST("{\"data\": [{\"sensor_id\": 1,\"sensor_data\":[{\"timestamp\": \"1676337423\",\"value\":\"46\"},{\"timestamp\": \"1676337400\",\"value\":\"46\"},{\"timestamp\": \"1676337350\",\"value\":\"70\"}]}]}");
      
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
    request = true;
}