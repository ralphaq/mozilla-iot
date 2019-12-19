bool var = 0 ;
const char* myssid = "ESP";
const char* mypassword = "12345678";
const char* host = "esp8266-webupdate";
char* ssid = "AndroidAP"; //not used
char* password = "098765432111";

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

  #include <Arduino.h>
  #include "Thing.h"
  #include "WebThingAdapter.h"

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
IPAddress local_ip(192,168,10,100);
IPAddress gateway(192,168,10,1);
IPAddress netmask(255,255,255,0);
  
  WebThingAdapter* adapter;
  const int ledPin = 13;
  const char* ledTypes[] = {"OnOffSwitch", "Light", nullptr};
  ThingDevice led("led", "Built-in LED", ledTypes);
  ThingProperty ledOn("on", "", BOOLEAN, "OnOffProperty");
  bool lastOn = false;

void setup(void) {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(9600);
  Serial.println("");
  Serial.print("Connecting to \"");
  Serial.print(ssid);
  Serial.println("\"");

  wifiConnect();
}

void loop() {
  // put your main code here, to run repeatedly:
         adapter->update();
          bool on = ledOn.getValue().boolean;
          digitalWrite(ledPin, on ? LOW : HIGH); // active low led
          if (on != lastOn) {
            Serial.print(led.id);
            Serial.print(": ");
            Serial.println(on);
          }
          lastOn = on;

 while (var == 1)
 {
  server.handleClient(); 
 }
}
void wifiConnect()
{
 WiFi.softAPdisconnect(true);
 WiFi.disconnect();          
 delay(1000);
 WiFi.mode(WIFI_STA);
 WiFi.begin(ssid, password);
      bool blink = true;
 unsigned long startTime = millis();
 while (WiFi.status() != WL_CONNECTED) 
        {
          delay(500);
          Serial.print(".");
                    digitalWrite(ledPin, blink ? LOW : HIGH); // active low led
                    blink = !blink;
         // digitalWrite(pin_led,!digitalRead(pin_led));
          if ((unsigned long)(millis() - startTime) >= 10000) break;
        }
 if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("connected in station mode");
                digitalWrite(ledPin, HIGH); // active low led
                Serial.println("");
                Serial.print("Connected to ");
                Serial.println(ssid);
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());
                adapter = new WebThingAdapter("w25", WiFi.localIP());
              
                led.addProperty(&ledOn);
                adapter->addDevice(&led);
                adapter->begin();
                Serial.println("HTTP server started");
                Serial.print("http://");
                Serial.print(WiFi.localIP());
                Serial.print("/things/");
                Serial.println(led.id);
    } else 
  {
    var =1 ;
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, netmask);
    WiFi.softAP(myssid, mypassword); 
    MDNS.begin(host);
    httpUpdater.setup(&server);
    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
    }
  Serial.println("");
  WiFi.printDiag(Serial);
}
