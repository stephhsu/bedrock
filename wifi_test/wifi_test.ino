#include <ESP8266WiFi.h>

const char *ssid =  "Cloudwifi-339-904";     // replace with your wifi ssid and wpa2 key
const char *pass =  "";

WiFiClient client;
 
void setup() 
{
       Serial.begin(9600);
       delay(10);
               
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
      Serial.println("NodeMCU IP address:");
      Serial.print(WiFi.localIP());
}
 
void loop() 
{      
  
}
