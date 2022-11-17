#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <stdio.h>
#include "secret.h" 
// comment above line, uncomment 2 lines below this one to have your login information here, instead of a seperate file
//#define SECRET_WIFI_SSID "SSID"
//#define SECRET_WIFI_PASSWORD "PASSWORD"
const char *WIFI_SSID = SECRET_WIFI_SSID;
const char *WIFI_PASSWORD = SECRET_WIFI_PASSWORD;

#define ARDUINOJSON_DECODE_UNICODE 1
#define TFT_CS    4     // TFT CS  pin is connected to NodeMCU pin D2
#define TFT_RST   0     // TFT RST pin is connected to NodeMCU pin D3
#define TFT_DC    2     // TFT DC  pin is connected to NodeMCU pin D4

// initialize ILI9341 TFT library with hardware SPI module
// SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
// MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Raspberri Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 1, 31)
#define MQTT_PORT 1883
#define MQTT_READTOPIC "esp/weatherstation/out" 
#define MQTT_PUBLISHTOPIC "esp/weatherstation/in"
//

// solarized dark colors
#define COLOR_BASE 0x0146
#define COLOR_TEXT 0x9514
#define COLOR_BLUE 0x245A
#define COLOR_ORANGE 0xCA42
#define COLOR_VIOLET 0x6B98
#define COLOR_RED 0xD000
#define COLOR_GREEN 0x84C0
#define COLOR_BLUE_2 0x8E99
#define COLOR_YELLOW 0xB440
#define COLOR_LIGHTBG 0xFFBC
//

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel pixels(1, 1, NEO_GRB + NEO_KHZ800);
Ticker watchdog, MQQTSender, blinkerWS;
int cycleCount = 0;  // counts cycles, if it doesn't change in interval/1000 seconds, you have a problem
int currentPWM = 0;
bool wsState = false;

void watchdogFunction() // "fixes" ESP occassionally hanging up
{
  if (!client.connected() || WiFi.status() != WL_CONNECTED)
  {
    ESP.restart();
  }
}

void setup() {
  WiFi.mode(WIFI_STA); // prevent old settings from setting any other mode
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(255, 255, 255));
  pixels.show();

  pinMode(5,OUTPUT);
  analogWrite(5,currentPWM); // testing pwm
  
  tft.begin(80000000);
  tft.setRotation(1);
  tft.fillScreen(COLOR_BASE);
  tft.setCursor(0, 0);
  tft.setTextSize(2);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    tft.print(".");
    delay(100);
    yield();
  }
  tft.println("");
  tft.println("Connected to WiFi network");
  // connect to MQQT, set callback
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(JSONPrint);
  while (!client.connected()) {
    tft.println("Connecting to MQTT...");
    if (client.connect("ESP32Client"))
    {
      tft.println("Connected to MQTT broker");
    }
  }
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
  
  watchdog.attach(120, watchdogFunction);
  MQQTSender.attach(60, requestMQTT);
  requestMQTT(); // fire off a message, so you don't have to wait a minute
}

void JSONPrintBars(char* topic, byte* json, unsigned int length)
{
  //vars - string title, minvalue, maxvalue, value, segmentcount, 
}
void JSONPrint(char* topic, byte* json, unsigned int length)
{
  const size_t capacity = 11 * JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(11) + 260;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    tft.fillScreen(ILI9341_RED);
    tft.setCursor(0, 0);
    tft.setTextColor(COLOR_BASE); tft.setTextSize(2);
    tft.print(error.c_str());
    return;
  }

  const char* key1 = doc[0][0]; // "Temperatura:"
  const char* value1 = doc[0][1]; // "4.22"

  const char* key2 = doc[1][0]; // "Wilgotnosc:"
  const char* value2 = doc[1][1]; // "87.21%"

  const char* key3 = doc[2][0]; // "PM25:"
  const char* value3 = doc[2][1]; // "110.02%"

  const char* key4 = doc[3][0]; // "PM10:"
  const char* value4 = doc[3][1]; // "90.86%"

  const char* key5 = doc[4][0]; // "Bojler:"
  const char* value5 = doc[4][1]; // "67.25"

  const char* key6 = doc[5][0]; // "Piec:"
  const char* value6 = doc[5][1]; // "54.94"

  const char* key7 = doc[6][0]; // "Raspberry Pi:"
  const char* value7 = doc[6][1]; // "35.5"

  const char* key8 = doc[7][0]; // "insidetemp"
  const char* value8 = doc[7][1]; // "26.30"

  const char* key9 = doc[8][0]; // "insidehumi"
  const char* value9 = doc[8][1]; // "60.50%"

  int key10 = doc[9][0]; // "led toggle on/off"
  int value10 = doc[9][1]; // "led brightness"

  int key11 = doc[10][0]; // "screen pwm"
  const char* value11 = doc[10][1]; // "placeholder"

  //tft.fillScreen(ILI9341_BLACK);
  tft.fillScreen(COLOR_BASE);
  tft.drawRect(0, 0, 319, 239, COLOR_VIOLET);
  tft.drawRect(1, 1, 317, 237, COLOR_VIOLET);
  tft.drawFastHLine(0, 131, tft.width(), COLOR_VIOLET);
  tft.drawFastVLine(165, 0, 131, COLOR_VIOLET);
  tft.setTextColor(COLOR_TEXT); tft.setTextSize(2);
  tft.setCursor(240, 106);
  tft.print(cycleCount);

  byte tableStartPos = 10;
  byte tableOffset = 16;
  TablePrint(tableStartPos, key1, value1);
  TablePrint(tableStartPos + tableOffset, key2, value2);
  TablePrint(tableStartPos + 2*tableOffset, key3, value3);
  TablePrint(tableStartPos + 3*tableOffset, key4, value4);
  tft.setTextColor(COLOR_ORANGE);
  TablePrint(tableStartPos + 4*tableOffset, key5, value5);
  TablePrint(tableStartPos + 5*tableOffset, key6, value6);
  TablePrint(tableStartPos + 6*tableOffset, key7, value7);
  PrintTemp(value8, value9);

  byte rectangleStartPos = 15;
  byte rectangleOffset = 50;
  ColorRectangle(rectangleStartPos, 18, 22, 26, 28, value8);//INSIDE TEMP
  ColorRectangle(rectangleStartPos + rectangleOffset, 40, 55, 65, 70, value9);//INSIDE HUMIDITY
  ColorRectangle(rectangleStartPos + 2*rectangleOffset, 20, 30, 40, 55, value5);//BOILER
  ColorRectangle(rectangleStartPos + 3*rectangleOffset, 20, 40, 50, 62, value6);//HEATING
  ColorRectangle(rectangleStartPos + 4*rectangleOffset, 0, 50, 100, 125, value3);//PM25
  ColorRectangle(rectangleStartPos + 5*rectangleOffset, 0, 50, 100, 125, value4);//PM10
  
  WS2812(key10, value10, value6);
  screenPWM(key11);
  //tft.print(key11);
  cycleCount++;
  return;
}

void TablePrint(int y_pos, const char* leftValue, const char* rightValue)
{
  tft.setCursor(10, y_pos);
  tft.print(leftValue);
  tft.setCursor(175, y_pos);
  tft.print(rightValue);
}

void PrintTemp(const char* temperature, const char* humidity)
{

  tft.setTextColor(COLOR_RED); 
  tft.setTextSize(4);
  tft.setCursor(30, 200);
  tft.print(temperature);
  tft.setTextColor(COLOR_BLUE);
  tft.setCursor(180, 200);
  tft.print(humidity);
  return;
}

void ColorRectangle(int x_pos, int low, int med, int high, int highAlarm, const char* inputValue) {
  double doubleValue = strtod(inputValue, NULL);
  if (doubleValue == 0.0) {
    tft.fillRoundRect(x_pos, 140, 40, 40, 6, ILI9341_MAGENTA);
  }
  else if (doubleValue > highAlarm) {
    //tft.fillRoundRect(x_pos, 140, 40, 40, 6, ILI9341_RED);
    tft.fillTriangle(x_pos, 180, x_pos+40, 180, x_pos+20, 140, ILI9341_RED);
    tft.fillRect(x_pos+18, 148, 5, 20, COLOR_BASE);
    tft.fillRect(x_pos+18, 172, 5, 5, COLOR_BASE);
  }
  else if (doubleValue > high) {
    tft.fillRoundRect(x_pos, 140, 40, 40, 6, ILI9341_YELLOW);
  }
  else if (doubleValue >= med) {
    tft.fillRoundRect(x_pos, 140, 40, 40, 6, ILI9341_GREEN);
  }
  else if (doubleValue < med && doubleValue > low) {
    tft.fillRoundRect(x_pos, 140, 40, 40, 6, ILI9341_BLUE);
  }
  else {
    tft.fillRoundRect(x_pos, 140, 40, 40, 6, COLOR_BASE);
  }
}


void WS2812(int toggle, int brightness, const char* variableValue) {
  blinkerWS.detach();
  if (toggle == 1) {
    double variable = strtod(variableValue, NULL);
    pixels.setBrightness(brightness);
    if (variable > 75) {
      blinkerWS.attach(1, blinkWS);
    }
    else if (variable > 60.0) {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));//red
    }
    else if (variable > 50.0) {
      pixels.setPixelColor(0, pixels.Color(255, 100, 0));//orange
    }
    else if (variable >= 26.0) {
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));//green
    }
    else if (variable < 26.0 && variable > 15.0) {
      pixels.setPixelColor(0, pixels.Color(0, 128, 255));//blue
    }
    else {
      pixels.setPixelColor(0, pixels.Color(255, 0, 255));//magenta
    }
    pixels.show();
  }
  else {
    pixels.setPixelColor(0, pixels.Color(1, 1, 1));
    pixels.show();
  }
}
void requestMQTT()
{
  // fire and forget. Node-RED is set up to send a JSON string after receiving a ping
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str())) {
    char pingmsg[] = "Ping!";
    // Once connected, publish an announcement...
    client.publish(MQTT_PUBLISHTOPIC, pingmsg);
    // ... and resubscribe
    client.subscribe(MQTT_READTOPIC);
  } else {
    // Wait 5 seconds before retrying
    delay(5000);
  }
}
void screenPWM(int newValue)
{
  if(currentPWM != newValue){
    currentPWM = newValue;
    analogWrite(5, newValue);
  }
}
void blinkWS()
{
  wsState = !wsState;
  if(wsState)
  { 
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  }
  else
  {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  }
  pixels.show();
}
void loop(void)
{
  client.loop();
  //keyboard code here. KB pins, 1-2-3-4: rx, d0, d6, rst 16 12
  // pwm pin - 5
}
