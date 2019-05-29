/**
A magic 8-ball like program using a Wemos D1 mini and a lolin e-ink display
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <LOLIN_EPD.h>
#include <Adafruit_GFX.h>

ESP8266WiFiMulti WiFiMulti;

/*D1 mini*/
#define EPD_CS D1
#define EPD_DC D8
#define EPD_RST -1  // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY -1 // can set to -1 to not use a pin (will wait a fixed delay)
#define EPD_MOSI D7
#define EPD_CLK D5
LOLIN_IL3897 EPD(250,122, EPD_MOSI, EPD_CLK, EPD_DC, EPD_RST, EPD_CS, EPD_BUSY); //IO

//Split using ";"
std::vector<String> splitResultToVector(String msg){
  std::vector<String> subStrings;
  int j=0;
  for(int i =0; i < msg.length(); i++){
    if(msg.charAt(i) == ';'){
      subStrings.push_back(msg.substring(j,i));
      j = i+1;
    }
  }
  subStrings.push_back(msg.substring(j,msg.length())); //to grab the last value of the string
  return subStrings;
  }

//Split using "\n"
std::vector<String> splitStringToVector(String msg, int* numberOfElements){
  std::vector<String> subStrings;
  int j=0;
  for(int i =0; i < msg.length(); i++){
    if(msg.charAt(i) == '\n'){
      subStrings.push_back(msg.substring(j,i));
      j = i+1;
      *numberOfElements += 1;
    }
  }
  subStrings.push_back(msg.substring(j,msg.length())); //to grab the last value of the string
  return subStrings;
  }

int numberOfElements = 0;
std::vector<String> guldkorn;

void setup() {
  EPD.begin();
  EPD.clearBuffer();
  EPD.setRotation(2);
  EPD.fillScreen(EPD_WHITE);
  EPD.setTextColor(EPD_BLACK);
  EPD.setTextSize(2);
  EPD.println("Getting new message, please wait!");
  EPD.display();
  
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "WIFIPASSWORD");

  while((WiFiMulti.run() != WL_CONNECTED)){
      
    Serial.println("Waiting for connection");
    delay(1000);
    }

  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin("http://www.myAwesomeURL.com/myTextfile.txt")) {  // HTTP. For HTTPS you add the fingerprint of the certificate as a new argument. I.e http.begin("URL", "1d XY AA AA")

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          //Serial.println(payload);

          guldkorn = splitStringToVector(payload, &numberOfElements); //split on each row
          
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
  delay(1000); //must wait for the screen to be ready
}


void loop() {
  int number = random(numberOfElements);
  EPD.clearBuffer();
  EPD.setCursor(2, 0);
  EPD.fillScreen(EPD_WHITE);
  EPD.setTextColor(EPD_BLACK);
  EPD.setTextSize(2);
  EPD.print(splitResultToVector(guldkorn[number])[0]);
  EPD.println(":");
  EPD.println(" ");
  EPD.setTextSize(1);
  String message = String(splitResultToVector(guldkorn[number])[1]);

  //an ugly way to replace åäö to correct characters.
  message.replace(String(char(229)), String(char(134))); //å
  message.replace(String(char(228)), String(char(132))); //ä
  message.replace(String(char(246)), String(char(148))); //ö
  message.replace(String(char(197)), String(char(143))); //Å
  message.replace(String(char(196)), String(char(142))); //Ä
  message.replace(String(char(214)), String(char(153))); //Ö

  EPD.println(message);
  EPD.display();
 
  ESP.deepSleep(5400000000); //deepsleep for ~90 minutes
}
