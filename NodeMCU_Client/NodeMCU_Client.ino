/*
  Sketch: NodeMCU_Client
  Description: NodeMCU Client for Collaborizm Mixpanel IoT project.
  Version: 1.0
  Author: Harsha Alva

  This sketch shows how to use and display Mixpanel data on a LCD using a NodeMCU.
  Please watch the Serial Monitor for debug info.

  For more details see: https://github.com/aharshac/Collaborizm_Mixpanel_IoT
*/

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>  // For I2C
#include <LiquidCrystal_I2C.h>  // For I2C LCD add-on control


const int FREE_RAM_THRESHOLD = 300; //  Reset board if RAM is below this amount

//  Intervals
const unsigned long DELAY_POLL_INTERVAL = 60000L; // Time between updates (ms)
const unsigned long DELAY_WAIT = 1000L; // Time to wait to show device status messages
const unsigned long DELAY_ERR = 2000L; // Time to wait after errors, before continuing
const unsigned long DELAY_ERR_LONG = 10000L; // Time to wait after bigger errors, before continuing

//  Data format helpers
const char CHR_CONTENT_START = char(30);    //  Server output end. ASCII code for record separator
//const char CHR_CONTENT_END = char(31);  //  Server output start.  ASCII code for unit separator

const unsigned long HTTP_TIMEOUT = 3000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response


//  WiFi config
#error "Change ASAP"
char gWiFiSsid[] = "";  // Wifi Network SSID (name)
char gWiFiPass[] = ""; // Wifi password


//  Server details
#error "Change ASAP"
char gServerName[] = "192.168.1.2";
const int gServerPort = 8970;
String gServerRestPath = "/events/arduino";  // Arduino/NodeMCU formatted OP


int gRunCount = 0;   // Run counter


/***  Expansions and shields ***/
LiquidCrystal_I2C LCD(0x27, 16, 2);  // I2C address for pins A4 (SDA), A5 (SCL); Set 16x2 LCD.


/*** Web Client ***/
WiFiClient WebClient;

void setup() {

  /*** Initialize Shields ***/
  LCD.init();
  LCD.clear();
  LCD.backlight();

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);  // Station Mode
  WiFi.persistent(false); // Don't save auth

  Serial.begin(115200); // Initialize UART serial for debugging

  checkMemState();
  serialOutF(F("Collaborizm Mixpanel IoT Project"));
  lcdOutF2(F("Collaborizm"), F("Mixpanel IoT"));
  delay(DELAY_WAIT * 2);  //
  /*** End Initialize Shields ***/


  /*** Connect to WiFi network ***/
  checkMemState();
  serialOutF1(F("Attempting to connect to WPA SSID"), gWiFiSsid);
  lcdOutF1(F("WiFi CXN INIT"), String(gWiFiSsid));
  delay(DELAY_WAIT * 1);  //

  boolean backlight = false; // Remeber old state to Toggle backlight while connecting

  WiFi.begin(gWiFiSsid, gWiFiPass);
  while (WiFi.status() != WL_CONNECTED) {     // Repeat till connected to WiFi network
      LCD.setBacklight(backlight);  // Toggle backlight while connecting
      backlight = !backlight;
      serialOutF(F("."));
      delay(DELAY_WAIT * 2);
  }
  LCD.backlight();  // Finally keep backlight on
  /*** End connect to WiFi network ***/


  /*** Output status and wait ***/
  checkMemState();
  serialOutF(F("Connected to wifi"));
  lcdOutF1(F("WiFi IP"), ipToString(WiFi.localIP()));
  outWiFiStats();
  delay(DELAY_WAIT);
  /*** End Output status and wait ***/
}

void loop(){
  fetchAndDisplayData();
}

void fetchAndDisplayData() {
  /*** Startup  ***/
  serialOutF(F("*** Loop Start ***"));
  /*** End Startup  ***/


  /*** Connect to server ***/
  checkMemState();

  // Send GET REQUEST
  if (WebClient.connect(gServerName, gServerPort)) {
    WebClient.println("GET " + gServerRestPath + " HTTP/1.1");
    WebClient.println("Host: " + String(gServerRestPath));
    WebClient.println("Connection: close");
    WebClient.println();
  } else {
    serialOutF(F("Connection failed"));
    lcdOutF2(F("Local Server"), F("CXN Failure"));
    windUpOnError();
    return;
  }


  serialOutF(F("Connected to Local Server"));
  lcdOutF2(F("Local Server"), F("CXN Success"));
  /*** End Connect to server ***/


  /*** Data Integrity check ***/
  checkMemState();

  WebClient.setTimeout(HTTP_TIMEOUT);
  bool ok = WebClient.find(CHR_CONTENT_START);
  if (!ok) {
    serialOutF(F("No data or invalid data received from Local Server. Retrying..."));
    lcdOutF2(F("Local Server"), F("RX data null"));
    windUpOnError();
    return;
  }

  /*** End Data Integrity check ***/


  /*** Start Data parse ***/
  checkMemState();

  char response[MAX_CONTENT_SIZE];
  size_t length = WebClient.readBytes(response, sizeof(response));
  response[length] = 0;
  serialOutF1(F("Server Response"), response);

  const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(2);  // the root object has 2 elements
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer; // Allocate a temporary memory pool on the stack
  JsonObject& root = jsonBuffer.parseObject(response);

  if (root.success()){
    const char *line0 = root["0"];
    const char *line1 = root["1"];

    lcdOutMpEvent(line0, line1);
    serialOutMpEvent(line0, line1);
  }else{
    serialOutF(F("Could not split Local Server data. Retrying..."));
    lcdOutF2(F("Local Server"), F("RX data invalid"));
    windUpOnError();
    return;
  }

  checkMemState();
  /*** End Data parse ***/

/*** Wind up ***/
  closeWebClient();
  checkMemState();
  serialOutF(F("*** Loop End ***\n"));
  ++gRunCount;  //  Increment run counter
/*** Wind up ***/

  delay(DELAY_POLL_INTERVAL); // Wait before next update check
}


/*** Garbage Collection ***/
void closeWebClient(){
  WebClient.flush(); //  Flush all previous received and transmitted data
  WebClient.stop();
}

void windUpOnError(){
  closeWebClient();
  delay(DELAY_ERR_LONG);
}
/*** End Garbage Collection ***/


/*** IO & Formatting  ***/

//  Output WiFi Station info to Serial
void outWiFiStats(){
  serialOutF1(F("SSID"), WiFi.SSID());

  IPAddress ip = WiFi.localIP();  //  IP address of ESP8266
  serialOutF1(F("IP Address"), ip);

  long rssi = WiFi.RSSI();  //  Signal strength
  serialOutF1(F("Signal strength (RSSI) (dBm)"), rssi);
  serialOutF(F(""));  // Empty line
}

//  Format IP address
String ipToString(const IPAddress &ip){
  if(!ip)
    return "IP not found";
  else
    return String(ip[0]) + String(".") + String(ip[1]) + String(".") + String(ip[2]) + String(".") + String(ip[3]);
}


//  Write Mixpanel event output to Serial.
void serialOutMpEvent(const char *line0, const char *line1){
  Serial.println("LCD Output:");
  Serial.println(line0);
  Serial.println(line1);
}

//  Write output to Serial. Low memory fix. Entire line is literal string
void serialOutF(const __FlashStringHelper *line){
  Serial.println(line);
}

//  Write output to Serial. Low memory fix. One string is literal
//  type T: New catch-all Type
template<typename T> void serialOutF1(const __FlashStringHelper *literal, T variable){
  Serial.print(literal);
  Serial.print((": "));
  Serial.println(variable);
}

//  Write output to LCD. Low memory fix. No string is literal
//  type T: New catch-all Type
void lcdOut(char *line0, char *line1){
  LCD.clear();
  delay(20);
  LCD.setCursor(0, 0);
  LCD.print(line0);
  delay(20);
  LCD.setCursor(0, 1);
  LCD.print(line1);
}

//  Write output to LCD. Low memory fix. No string is literal
//  type T: New catch-all Type
void lcdOutMpEvent(const char *line0, const char *line1){
  LCD.clear();
  delay(20);
  LCD.setCursor(0, 0);
  LCD.print(line0);
  delay(20);
  LCD.setCursor(0, 1);
  LCD.print(line1);
}

//  Write output to LCD. Low memory fix. One string is literal
//  type T: New catch-all Type
template<typename T> void lcdOutF1(const __FlashStringHelper *line0, T line1){
  LCD.clear();
  delay(20);
  LCD.setCursor(0, 0);
  LCD.print(line0);
  delay(20);
  LCD.setCursor(0, 1);
  LCD.print(line1);
}

//  Write output to LCD. Low memory fix. Both strings are literal
void lcdOutF2(const __FlashStringHelper *line0, const __FlashStringHelper *line1){
  LCD.clear();
  delay(20);
  LCD.setCursor(0, 0);
  LCD.print(line0);
  delay(20);
  LCD.setCursor(0, 1);
  LCD.print(line1);
}
/*** End IO & Formatting  ***/


/*** WDT & Reset ***/
//  FRAM = free RAM
//  CRUN = complete executions of loop. 0 index.
void checkMemState(){
  int freeRam = ESP.getFreeHeap();
  String strFreeRam = String(freeRam);
  String strRunCount = String(gRunCount);

  //  Low memory. Reset NodeMCU
  if(freeRam < FREE_RAM_THRESHOLD){
    serialOutF1(F("MEM CHK: RST RAM LOW. FRAM CRUN"), strFreeRam + " " + strRunCount);
    lcdOutF1(F("RST: FMEM; RUN"), strFreeRam + "; " + strRunCount);
    ESP.restart();  // Restart NodeMCU
  }else{
    serialOutF1(F("MEM CHK: FRAM CRUN"), strFreeRam + " " + strRunCount);
  }
}
/*** End WDT & Reset  ***/
