/**
 * Simple server compliant with Mozilla's proposed WoT API
 * Originally based on the HelloServer example
 * Tested on ESP8266, ESP32, Arduino boards with WINC1500 modules (shields or
 * MKR1000)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Arduino.h>
#include "Thing.h"
#include "WebThingAdapter.h"

//TODO: Hardcode your wifi credentials here (and keep it private)
const char* ssid = "iot";
const char* password = "iot@12345";

#if defined(LED_BUILTIN)
const int ledPin = LED_BUILTIN;
const int ledPin1 = D1;
#else
const int ledPin = D0;  // manually configure LED pin
const int ledPin1 = D1;
#endif

WebThingAdapter* adapter;

const char* ledTypes[] = {"OnOffSwitch", "Light", nullptr};
ThingDevice led("led", "Built-in LED", ledTypes);
ThingProperty ledOn("on", "", BOOLEAN, "OnOffProperty");
ThingDevice led1("led1", "Built-in LED", ledTypes);
ThingProperty ledOn1("on1", "", BOOLEAN, "OnOffProperty");

bool lastOn = false;
bool lastOn1 = false;

void setup(void){
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
   pinMode(ledPin1, OUTPUT);
  digitalWrite(ledPin1, HIGH);
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("");
  Serial.print("Connecting to \"");
  Serial.print(ssid);
  Serial.println("\"");
#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  bool blink = true;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(ledPin, blink ? LOW : HIGH); // active low led
       digitalWrite(ledPin1, blink ? LOW : HIGH); // active low led
    blink = !blink;
  }
  digitalWrite(ledPin, HIGH); // active low led
  digitalWrite(ledPin1, HIGH); // active low led
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  adapter = new WebThingAdapter("w25", WiFi.localIP());

  led.addProperty(&ledOn);
  adapter->addDevice(&led);
   led1.addProperty(&ledOn1);
  adapter->addDevice(&led1);
  adapter->begin();
  Serial.println("HTTP server started");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(led.id);
}

void loop(void){
  adapter->update();
  bool on = ledOn.getValue().boolean;
  digitalWrite(ledPin, on ? HIGH : LOW); // active low led
   bool on1 = ledOn1.getValue().boolean;
  digitalWrite(ledPin1, on1 ? HIGH : LOW); // active low led
  if (on != lastOn) {
    Serial.print(led.id);
    Serial.print(": ");
    Serial.println(on);
  }
   if (on1 != lastOn1) {
    Serial.print(led1.id);
    Serial.print(": ");
    Serial.println(on1);
  }
  lastOn = on;
   lastOn1 = on1;
}
