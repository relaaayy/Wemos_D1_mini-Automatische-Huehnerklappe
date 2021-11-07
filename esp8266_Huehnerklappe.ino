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
#define Photoresistor A0
#define relayPin_down 16
#define relayPin_up 14

AsyncWebServer server(80);

const char* PARAM_INPUT_1 = "input1";
const int analogPin = A0; 
const int ledPin = 2;
const int switchbrightness = 25;
int delay_shut = 300;
bool state = true;
bool state_roller = true;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>H&uuml;hnerklappe</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Schlie&szlig;zeit in Sekunden: <input type="text" name="input1">
    <input type="submit" value="Speichern">
  </form><br>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(9600);  // set baud rate to 
  pinMode(relayPin_down, OUTPUT);
  pinMode(relayPin_up, OUTPUT);
  pinMode(ledPin, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println("Change to: " + inputMessage);
    delay_shut = inputMessage.toInt()* 1000;
    
    request->send(200, "text/html", "erfolgreich gespeichert (" 
                                     + inputParam + ") Schlie&szlig;zeit: " + inputMessage +
                                     "<br><a href=\"/\">zur&uuml;ck</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  int analogValue = analogRead(Photoresistor);
  int brightness = map(analogValue, 0, 1000, 0, 100);

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
        Serial.println("Nothing cause no Changes");
        }
}
