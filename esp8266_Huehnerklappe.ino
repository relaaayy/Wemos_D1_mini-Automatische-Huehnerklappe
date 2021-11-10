// Automatic Chickendoor
// Author: relaychris
// Github: https://github.com/relaychris/ 
// Version: 0.1
// Creds: https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/

//  ######   #######  ####       ##     ##  ##     ####   ##   ##  ######    ####     #####
//   ##  ##   ##   #   ##       ####    ##  ##    ##  ##  ##   ##   ##  ##    ##     ##   ##
//   ##  ##   ## #     ##      ##  ##   ##  ##   ##       ##   ##   ##  ##    ##     #
//   #####    ####     ##      ##  ##    ####    ##       #######   #####     ##      #####
//   ## ##    ## #     ##   #  ######     ##     ##       ##   ##   ## ##     ##          ##
//   ##  ##   ##   #   ##  ##  ##  ##     ##      ##  ##  ##   ##   ##  ##    ##     ##   ##
//  #### ##  #######  #######  ##  ##    ####      ####   ##   ##  #### ##   ####     #####

#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "wlan_settings.h"

//Photoresistor to Port A0
#define Photoresistor A0
//Relay Pin to Port D0
#define relayPin_down 16
//Relay Pin to Port D5
#define relayPin_up 14

AsyncWebServer server(80);

const char* PARAM_INPUT = "input";
const int analogPin = A0; 
const int ledPin = 2;
const int switchbrightness = 25;
int delay_shut = 300;
bool state = true;
bool state_roller = true;

//HTMl Page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>H&uuml;hnerklappe</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Schlie&szlig;zeit in Sekunden: <input type="text" name="input">
    <input type="submit" value="Speichern">
  </form><br>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(9600);  // set baud rate to 9600 
  pinMode(relayPin_down, OUTPUT); // set PinMode for Relay
  pinMode(relayPin_up, OUTPUT); // set PinMode for Relay
  pinMode(ledPin, OUTPUT); // set ledPin for Relay
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Check WiFi Connection and Print status
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  // Print Ip Adress
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input value on <ESP_IP>/get?input=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      inputParam = PARAM_INPUT;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println("Change to: " + inputMessage);

    // Convert Input to int and change it from s to ms
    delay_shut = inputMessage.toInt() * 1000;
    
    request->send(200, "text/html", "erfolgreich gespeichert (" 
                                     + inputParam + ") Schlie&szlig;zeit: " + inputMessage +
                                     "<br><a href=\"/\">zur&uuml;ck</a>");
  });
  server.onNotFound(notFound);
  // Start Webserver
  server.begin();
}

void loop() {
  // Read Analog input from Photoresitstor
  int analogValue = analogRead(Photoresistor);
  int brightness = map(analogValue, 0, 1000, 0, 100);
  
  // Check Brightness and trigger Relay
  if (brightness > switchbrightness) {
    rollershutter();
    state = false;
    Serial.println(brightness);
    delay(100);
  } 
  else if (brightness < switchbrightness) {
    rollershutter();
    state = true;
    Serial.println(brightness);
    delay(100);
  }
  
 delay(8000);
}

void rollershutter() {
    if(state == false && state_roller == false) {
      digitalWrite(ledPin, HIGH);
      digitalWrite(relayPin_down, LOW);
      digitalWrite(relayPin_up, HIGH);
      delay(delay_shut);
      digitalWrite(relayPin_up, LOW);
      Serial.println("up");
      state_roller = true;
      }
      else if(state == true  && state_roller == true) {
      digitalWrite(ledPin, LOW);
      digitalWrite(relayPin_down, HIGH);
      digitalWrite(relayPin_up, LOW);
      delay(delay_shut);
      digitalWrite(relayPin_down, LOW);
      Serial.println("down");
      state_roller = false;
      }
      else {
        Serial.println("Nothing todo, cause no Changes");
        }
}
