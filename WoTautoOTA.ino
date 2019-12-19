#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

IPAddress local_ip(192,168,10,100);
IPAddress gateway(192,168,10,1);
IPAddress netmask(255,255,255,0);

const char* myssid = "ESP";
//const char* mypassword = "";
const char* host = "esp8266-webupdate";
char* ssid = "iot"; //not used
char* password = "iot@12345";

AsyncWebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

bool restartRequired = false;  // Set this flag in the callbacks to restart ESP in the main loop

#include <Arduino.h>
#include "Thing.h"
#include "WebThingAdapter.h"

#if defined(LED_BUILTIN)
const int ledPin = LED_BUILTIN;
#else
const int ledPin = D0;  // manully configure LED pin
#endif

WebThingAdapter* adapter;

const char* ledTypes[] = {"OnOffSwitch", "Light", nullptr};
ThingDevice led("led", "Built-in LED", ledTypes);
ThingProperty ledOn("on", "", BOOLEAN, "OnOffProperty");

bool lastOn = false;


void setup(void) {
pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
Serial.begin(9600);
Serial.println("");
wifiConnect();

server.begin();

}

void loop(void){
//    if (restartRequired){  // check the flag here to determine if a restart is required
//    Serial.printf("Restarting ESP\n\r");
//    restartRequired = false;
//    ESP.restart();
//  }
  adapter->update();
  bool on = ledOn.getValue().boolean;
  digitalWrite(ledPin, on ? LOW : HIGH); // active low led
  if (on != lastOn) {
    Serial.print(led.id);
    Serial.print(": ");
    Serial.println(on);
  }
  lastOn = on;
}
void wifiConnect()
{
 WiFi.softAPdisconnect(true);
 WiFi.disconnect();          
 delay(1000);
 WiFi.mode(WIFI_STA);
 WiFi.begin(ssid, password);
 unsigned long startTime = millis();
  bool blink = true;
 while (WiFi.status() != WL_CONNECTED) 
        {
          delay(500);
          Serial.print(".");
          digitalWrite(ledPin, blink ? HIGH : LOW); // active low led
    blink = !blink;
         // digitalWrite(pin_led,!digitalRead(pin_led));
          if ((unsigned long)(millis() - startTime) >= 10000) break;
        }

 if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("connected in station mode");
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
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, netmask);
    WiFi.softAP(myssid, password); 

       
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", serverIndex);
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    // the request handler is triggered after the upload has finished... 
    // create the response, add header, and send response
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError())?"FAIL":"OK");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    restartRequired = true;  // Tell the main loop to restart the ESP
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    //Upload handler chunks in data
    
    if(!index){ // if index == 0 then this is the first frame of data
      Serial.printf("UploadStart: %s\n", filename.c_str());
      Serial.setDebugOutput(true);
      
      // calculate sketch space required for the update
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if(!Update.begin(maxSketchSpace)){//start with max available size
        Update.printError(Serial);
      }
      Update.runAsync(true); // tell the updaterClass to run in async mode
    }

    //Write chunked data to the free sketch space
    if(Update.write(data, len) != len){
        Update.printError(Serial);
    }
    
    if(final){ // if the final flag is set then this is the last frame of data
      if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u B\nRebooting...\n", index+len);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  
   // MDNS.begin(host);
   // httpUpdater.setup(&server);
   // MDNS.addService("http", "tcp", 80);
   // Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
    }
  Serial.println("");
  WiFi.printDiag(Serial);
}
