#include <ESP8266WiFi.h>

const char *ssid =  "Cloudwifi-339-904";
const char *pass =  "";
bool isConnectedToWifi = false;

WiFiClient client;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for the serial port to connect. needed for native usb port
  }
}

void loop() {
  // TODO: wait for signal from rover_data
  if (!isConnectedToWifi) {
    connect_to_wifi();
    isConnectedToWifi = true;
  } else {
    if (Serial.available()) {
      // this prints to serial monitor
      Serial.write(Serial.read());
      
      // TODO: send data to database instead
    }
    
  }

  // once rover is away from the homebase, isConnectedToWifi = false
}


void connect_to_wifi() {
  Serial.println("Connecting to ");
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("NodeMCU IP address:");
  Serial.print(WiFi.localIP());
}
