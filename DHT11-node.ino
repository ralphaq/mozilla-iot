/**
 * Publishing DHT temperature and humidity via the Web Thing API.
 *
 * This should work with DHT11, DHT21, DHT22 alike thanks to
 * device auto detection in the markruys/arduino-DHT library.
 *
 * This will make the device available under http://livingroom.local/
 * for msdns-aware clients.
 * 
 * Dependencies:
 * - https://github.com/markruys/arduino-DHT
 * - https://github.com/mozilla-iot/webthing-arduino
 *
 * Devices:
 * - DHT sensor connected with black=GND, red=5V or 3.3V, yellow=D5
 */

#include "Thing.h"
#include "WebThingAdapter.h"
#include "DHT.h"

const char* ssid = "iot";
const char* password = "iot@12345";

const int ledPin = LED_BUILTIN;

// minimal change from last sensor reading to
// update our values
const double threshold = 0.1;

WebThingAdapter* adapter;

// MultiLevelSensor and Sensor are mandatory for this type of application.
// See https://iot.mozilla.org/schemas/
const char* deviceTypes[] = {"MultiLevelSensor", "Sensor", nullptr};

// Sets up the device info. First two values are free-form.
ThingDevice device("DHTSensor", "Living room temperature and humidity sensor", deviceTypes);

// Sets up device properties. Not sure about the second value here. Probably doesn't make sense.
// Third and fourth argument are not freeform.
ThingProperty humidity("humidity", "Input pin", NUMBER, "LevelProperty");
ThingProperty temperature("temperature", "Input pin", NUMBER, "LevelProperty");

// Pin to connect DHT sensor's data wire (yellow)
const int sensorPin = D5;
DHT dht;
int samplingPeriod = dht.getMinimumSamplingPeriod();

double lastHumidityValue = 0;
double lastTemperatureValue = 0;

void setup(void){
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(115200);
  Serial.println("");

  dht.setup(sensorPin);
  Serial.print("Sensor model: ");
  Serial.println(dht.getModel());
  
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
    blink = !blink;
  }
  digitalWrite(ledPin, HIGH); // active low led

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Here 'livingroom' becomes part of the hostname => livingroom.local
  // so make sure it's unique in your network.
  adapter = new WebThingAdapter("livingroom", WiFi.localIP());

  device.addProperty(&humidity);
  device.addProperty(&temperature);
  adapter->addDevice(&device);

  Serial.println("Starting HTTP server");
  adapter->begin();
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(device.id);
}

// The loop function makes sure we get up-to-date values.
void loop(void){
  delay(samplingPeriod);
    
  float humidityVal = dht.getHumidity();
  double humidityValDouble = (double) humidityVal;
  if (abs(humidityValDouble - lastHumidityValue) >= threshold) {
    Serial.print("log: Humidity: ");
    Serial.print(humidityValDouble);
    Serial.println("%");
    ThingPropertyValue humidityLevelValue;
    humidityLevelValue.number = humidityValDouble;
    humidity.setValue(humidityLevelValue);
    lastHumidityValue = humidityValDouble;
  }
  adapter->update();

  delay(samplingPeriod);
  
  float temperatureVal = dht.getTemperature();
  double temperatureValDouble = (double) temperatureVal;
  if (abs(temperatureValDouble - lastTemperatureValue) >= threshold) {
    Serial.print("log: Temperature: ");
    Serial.print(temperatureValDouble);
    Serial.println("°C");
    ThingPropertyValue temperatureLevelValue;
    temperatureLevelValue.number = temperatureValDouble;
    temperature.setValue(temperatureLevelValue);
    lastTemperatureValue = temperatureValDouble;
  }
  adapter->update();
}
