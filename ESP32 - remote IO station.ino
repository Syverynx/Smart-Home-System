#include <Adafruit_GFX.h> 
#include <Adafruit_ST7735.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
 
// ST7735 TFT module connections
#define TFT_RST   4     // TFT RST pin is connected to NodeMCU pin D4 (GPIO2)
#define TFT_CS    5     // TFT CS  pin is connected to NodeMCU pin D3 (GPIO0)
#define TFT_DC    2     // TFT DC  pin is connected to NodeMCU pin D2 (GPIO4)
const int buttonDown = 14;  // pushbutton pin
const int buttonSelect = 12;  // pushbutton pin
const int buttonUp = 13;  // pushbutton pin
// initialize ST7735 TFT library with hardware SPI module
// SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
// MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// solarized dark colors
#define COLOR_BASE 0x0146
#define COLOR_TEXT 0x9514
#define COLOR_BLUE 0x245A
#define COLOR_ORANGE 0xCA42
#define COLOR_VIOLET 0x6B98
#define COLOR_RED 0xD000

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define MQTT_HOST IPAddress(192, 168, 1, 31)
#define MQTT_PORT 1883
#define MQTT_READTOPIC "esp/megastation/out"
#define MQTT_PUBLISHTOPIC "esp/megastation/in"

WiFiClient espClient;
PubSubClient client(espClient);

int pos = 0;
int state = 1;
char argarray[4][16] = 
{
  ("WEATHER"),
  ("Pi OFF"),
  ("PC OFF"),
  ("Router OFF")
};

int buttonDownState = 0;
int buttonSelectState = 0;
int buttonUpState = 0;

void setup(void)
{
  Serial.begin(115200);
  tft.initR(INITR_BLACKTAB);     // initialize a ST7735S chip, black tab
  tft.fillScreen(COLOR_BASE);  // fill screen with black color
  tft.setRotation(1);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  pinMode(buttonDown, INPUT);
  pinMode(buttonSelect, INPUT);
  pinMode(buttonUp, INPUT);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  tft.println("Connecting to Wi-Fi");  

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    tft.print(".");  
  }
  tft.println("");
  tft.println("Connected!");


  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
  
  while (!client.connected()) {
    tft.println("Connecting to MQTT...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected to MQTT broker");
      tft.println("Connected to MQTT broker");
    }
    else
    {
      tft.print("failed with state ");
      tft.print(client.state());
      delay(500);
    }
  } 
  client.subscribe(MQTT_READTOPIC);
  
  //tft.drawFastHLine(0, 50,  tft.width(), COLOR_ORANGE);   // draw horizontal blue line at position (0, 50)
  //tft.drawFastHLine(0, 102,  tft.width(), COLOR_ORANGE);  // draw horizontal blue line at position (0, 102)
  menuInit();
}


/* menu for 8 tiles
void menu()
{
  tft.drawRect(0, 0, tft.width(), 16, COLOR_ORANGE);
  tft.drawRect(0, 16, tft.width(), 16, COLOR_ORANGE);
  tft.drawRect(0, 32, tft.width(), 16, COLOR_ORANGE);
  tft.drawRect(0, 48, tft.width(), 16, COLOR_ORANGE);
  tft.drawRect(0, 64, tft.width(), 16, COLOR_ORANGE);
  tft.drawRect(0, 80, tft.width(), 16, COLOR_ORANGE);
  tft.drawRect(0, 96, tft.width(), 16, COLOR_ORANGE);
  tft.drawRect(0, 112, tft.width(), 16, COLOR_ORANGE);
}

void menuPick(int pos)
{
  menu();
  previouspos = pos-1;
  tft.fillRect(1, pos*16, tft.width()-2, 15, COLOR_BLUE);
  tft.setCursor(32, 4 + (pos*16));
  tft.print(argarray[pos]);
  clearPrevious(previouspos);
  return;
}
void clearPrevious(int pos)
{
    tft.fillRect(1, pos*16, tft.width()-2, 15, COLOR_BASE);
    return;
}
*/
void menuInit()
{
  tft.fillScreen(COLOR_BASE);
  tft.setTextColor(COLOR_TEXT);
  pos = 0;
  state = 1;
  menuDraw(0);
  menuPick(0);
  menuDraw(1);
  menuDraw(2);
  menuDraw(3); 
  return;
}
void menuDraw(int pos)
{
  tft.drawRect(0, pos*32, tft.width(), 32, COLOR_ORANGE);
  tft.setCursor(16, 8 + (pos*32));
  tft.setTextSize(2);
  tft.print(argarray[pos]);
  return;
}
void menuPick(int pos)
{
  tft.fillRect(2, 1+(pos*32), tft.width()-4, 30, COLOR_BLUE);
  tft.setCursor(16, 8 + (pos*32));
  tft.setTextSize(2);
  tft.print(argarray[pos]);
  return;
}
void clearPrevious(int pos)
{
  tft.fillRect(1, 1+(pos*32), tft.width()-4, 30, COLOR_BASE);
  tft.setCursor(16, 8 + (pos*32));
  tft.setTextSize(2);
  tft.print(argarray[pos]);
  return;
}

void invokeMethod(int pos)
{
  switch(pos)
  {
    case 0:
     state = 3;
     PublishMessage(argarray[pos], true);
     break;
     
    case 1:
     ConfirmMQQT();
     break;
     
     case 2:
     ConfirmMQQT();
     break;
     
     case 3:
     ConfirmMQQT();
     break;
     
     default:
     menuInit();
     pos = 0;
  }
}

void readButtons()
{
  buttonDownState = digitalRead(buttonDown);
  buttonSelectState = digitalRead(buttonSelect);
  buttonUpState = digitalRead(buttonUp);
  //delay(20);
}

void switchTest()
{
  switch(state)
  {
    case 1:
    if(buttonUpState == 1)
    {
        clearPrevious(pos);
        menuPick(pos-1);
        pos--;
        delay(250);       
    }
    if(buttonSelectState == 1)
    {
      invokeMethod(pos);
    }
    if(buttonDownState == 1)
    {
      if(pos != 3)
      {
        clearPrevious(pos);
        menuPick(pos+1);
        pos++;
        delay(250); 
      }     
    }
    break;
    
    case 2:
    if(buttonSelectState == 1)
    {
      PublishMessage(argarray[pos], false);
      ConfirmedMQQT();
    }
    break;
    
    case 3:
    if(buttonSelectState == 1)
    {
      delay(250); 
      menuInit();      
    }
    break;
  }
  
}

void ConfirmMQQT()
{
  state = 2;
  tft.fillScreen(COLOR_BASE);
  tft.setTextColor(COLOR_RED);
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.println("Confirm?");
  tft.println();
  tft.setTextSize(2);
  tft.setTextColor(COLOR_VIOLET);
  tft.println(argarray[pos]);
  delay(1000);
}
void ConfirmedMQQT()
{
  if(state = 2)
  {
  tft.fillScreen(COLOR_BASE);
  tft.setTextColor(COLOR_ORANGE);
  tft.setTextSize(3);
  tft.setCursor(20, 32);
  tft.println("MESSAGE");
  tft.setCursor(36, 56);
  tft.println("SENT");

  tft.drawRect(0, 0, tft.width(), tft.height(), COLOR_BLUE);
  delay(250);

  tft.drawRect(5, 5, tft.width()-10, tft.height()-10, COLOR_BLUE);
  delay(250);

  tft.drawRect(10, 10, tft.width()-20, tft.height()-20, COLOR_BLUE);
  delay(250);

  tft.drawRect(15, 15, tft.width()-30, tft.height()-30, COLOR_BLUE);
  delay(250);

  menuInit();
  }

}

void JSONPrint(String json)
{
  int pos = 80;
  const size_t capacity = 8*JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(8) + 240;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, json);

  const char* label1 = doc[0][0]; // "Key 1"
  const char* value1 = doc[0][1]; // "Value 1"

  const char* label2 = doc[1][0]; // "Key 2"
  const char* value2 = doc[1][1]; // "Value 2"

  const char* label3 = doc[2][0]; // "Key 3"
  const char* value3 = doc[2][1]; // "Value 3"

  const char* label4 = doc[3][0]; // "Key 4"
  const char* value4 = doc[3][1]; // "Value 4"

  const char* label5 = doc[4][0]; // "Key 5"
  const char* value5 = doc[4][1]; // "Value 5"

  const char* label6 = doc[5][0]; // "Key 6"
  const char* value6 = doc[5][1]; // "Value 6"

  const char* label7 = doc[6][0]; // "Key 7"
  const char* value7 = doc[6][1]; // "Value 7"

  const char* label8 = doc[7][0]; // "B-TEMP"
  const char* value8 = doc[7][1]; // "42.81"


  tft.fillScreen(COLOR_BASE);
  tft.setTextColor(COLOR_ORANGE);
  tft.setTextSize(1);
  printLine(0, pos, label1, value1);
  printLine(1, pos, label2, value2);
  printLine(2, pos, label3, value3);
  printLine(3, pos, label4, value4);
  printLine(4, pos, label5, value5);
  printLine(5, pos, label6, value6);
  printLine(6, pos, label7, value7);
  printLine(7, pos, label8, value8);
  state = 3;
  delay(1000);
}

void printLine(unsigned int position, int lineposition, const char* leftline, const char* rightline)
{
    // prints key-value pairs, works with ArialMT_Plain_10 as font. Changing sizes will require changing the position multiplier
    tft.setCursor(0, position*16);
    tft.setTextColor(COLOR_BLUE);
    tft.println(leftline);    
    tft.setCursor(lineposition, position*16);
    tft.setTextColor(COLOR_TEXT);
    tft.println(rightline);
    return;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String json = (char*)payload;  //forgot to test if it works fine if you pass just the char*
  JSONPrint(json);
  return;
}

void PublishMessage(char message[16], bool subscribeToMQQT) {
    // fire and forget
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) 
    {
      // Once connected, publish an announcement...
      client.publish(MQTT_PUBLISHTOPIC, message);
      
      if (subscribeToMQQT == true)
      {
        client.subscribe(MQTT_READTOPIC);
        state = 3;
      }          
    } 
    else 
    {
      // Wait 5 seconds before retrying
      delay(5000);
    }
}

// main loop
void loop()
{ 
  client.loop();
  readButtons();
  switchTest(); 
}
