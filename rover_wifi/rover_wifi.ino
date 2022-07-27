#include <ESP8266WiFi.h>

const char *ssid =  "Cloudwifi-339-904";
const char *pass =  "";
const char homebase = 'H';
const char notHomebase = 'N';
bool isConnectedToWifi = false;
bool atHomebase = false;

WiFiClient client;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for the serial port to connect. needed for native usb port
  }
}

void loop() {
  // signal that we are at homebase
  if (Serial.available() && (Serial.read()==homebase)) {
    atHomebase = true;
  }

  // we have left homebase
  if (Serial.available() && (Serial.read()==notHomebase)) {
    atHomebase = false;
    isConnectedToWifi = false;
  }

  if (atHomebase) {
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
  }
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
