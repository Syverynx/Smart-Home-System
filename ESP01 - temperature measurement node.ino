/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-mqtt-publish-ds18b20-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <Adafruit_NeoPixel.h>
const char *WIFI_SSID = SECRET_WIFI_SSID;
const char *WIFI_PASSWORD = SECRET_WIFI_PASSWORD;

// Raspberri Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 1, 31)
// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
#define MQTT_PORT 1883

// Temperature MQTT Topics
#define MQTT_PUB_TEMP "esp/ds18b20/temperature"

// GPIO where the DS18B20 is connected to
const int oneWireBus = 2;    
    
Adafruit_NeoPixel pixels(1, 3, NEO_GRB + NEO_KHZ800);// one led on gpio 3
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
// Temperature value
float temp;
bool ledState;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 60000;        // Interval at which to publish sensor readings

unsigned long previousMillisLED = 0;
const long intervalLED = 500;

void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

/*void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}*/

void onMqttPublish(uint16_t packetId) {
}

void setup() {
  sensors.begin();
  pixels.begin();
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  //mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  //mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  pixels.setBrightness(70);
  pixels.setPixelColor(0, pixels.Color(255, 255, 255));
  pixels.show();
  connectToWifi();
}

void MQQTPublish()
{
    unsigned long currentMillis = millis();
  // Every X number of seconds (interval = 10 seconds) 
  // it publishes a new MQTT message
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    // New temperature readings
    sensors.requestTemperatures(); 
    // Temperature in Celsius degrees
    temp = sensors.getTempCByIndex(0);    
    // Publish an MQTT message on topic esp/ds18b20/temperature
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 2, true, String(temp).c_str()); 
    LED();                           
  }
}

void LED(){
  if(temp > 52 && temp < 85)
  {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));//red
  }
  else if(temp > 37){
    pixels.setPixelColor(0, pixels.Color(255, 40, 0));//orange
  }
  else if(temp >= 26){
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));//green
  }
  else if(temp < 26 && temp > 0){
    pixels.setPixelColor(0, pixels.Color(0, 128, 255));//blue
  }
  else{
    pixels.setPixelColor(0, pixels.Color(255, 0, 255));//magenta
  }
  pixels.show();
}
void loop() 
{
  MQQTPublish();
}
