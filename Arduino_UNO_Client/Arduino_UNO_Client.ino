/*
  Sketch: Arduino_UNO_Client
  Description: Arduino UNO Client for Collaborizm Mixpanel IoT project.
  Version: 1.0
  Author: Harsha Alva

  This sketch shows how to use and display Mixpanel data on a LCD using an Arduino.
  Please watch the Serial Monitor for debug info.

  For more details see: https://github.com/aharshac/Collaborizm_Mixpanel_IoT
*/

#include <avr/wdt.h>  // For WDT reset
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <WiFiEsp.h>


const int FREE_RAM_THRESHOLD = 300; //  Reset board if RAM is below this amount

//  Intervals
const unsigned long DELAY_POLL_INTERVAL = 60000L; // Time between updates (ms)
const unsigned long DELAY_WAIT = 1000L; // Time to wait to show device status messages
const unsigned long DELAY_ERR = 2000L; // Time to wait after errors, before continuing
const unsigned long DELAY_ERR_LONG = 10000L; // Time to wait after bigger errors, before continuing

//  Data format helpers
const char CHR_CONTENT_START = char(30);    //  Server output end. ASCII code for record separator
//const char CHR_CONTENT_END = char(31);  //  Server output start.  ASCII code for unit separator

//  Pin config
const int pinEspRx = 2;  //  Esp Rx <----> Arduino Tx
const int pinEspTx = 3;  //  Esp Tx <----> Arduino Rx
const int pinLcdRs = 15;  //  LCD RS
const int pinLcdE = 14;   //  LCD E
const int pinLcdD0 = 4;  //  LCD D0
const int pinLcdD1 = 5;  //  LCD D1
const int pinLcdD2 = 6;  //  LCD D2
const int pinLcdD3 = 7;  //  LCD D3
const int pinLcdD4 = 8;  //  LCD D4
const int pinLcdD5 = 9;  //  LCD D5
const int pinLcdD6 = 10;  //  LCD D6
const int pinLcdD7 = 11;  //  LCD D7


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
String gServerRestPath = "/events/arduino";  // Arduino formatted OP


int gRunCount = 0;   // Run counter


/***  Expansions and shields ***/
SoftwareSerial ESP8266(pinEspTx, pinEspRx);   // ESP8266 Wifi Module
//LiquidCrystal LCD(pinLcdRs, pinLcdE, pinLcdD4, pinLcdD5, pinLcdD6, pinLcdD7); // Init serial
LiquidCrystal LCD(pinLcdRs, pinLcdE, pinLcdD0, pinLcdD1, pinLcdD2, pinLcdD3, pinLcdD4, pinLcdD5, pinLcdD6, pinLcdD7); // Init parallel


/*** Web Client ***/
WiFiEspClient WebClient;

void setup() {
  wdtResetDisable();  // Disable WDT ASAP

  /*** Initialize Shields ***/
  Serial.begin(9600); // Initialize UART serial for debugging
  checkMemState();
  ESP8266.begin(9600);  // Initialize Software UART serial for ESP module
  LCD.begin(16,2);  //  Initialize 16x2 LCD
  lcdOutF2(F("Collaborizm"), F("Arduino-Mixpanel"));
  /*** End Initialize Shields ***/


  /*** Detect and Start ESP-01 ***/
  WiFi.init(&ESP8266); // initialize ESP module
  checkMemState();
  if (WiFi.status() == WL_NO_SHIELD) {  // Check for the presence of the ESP
    serialOutF(F("ESP WiFi shield not present"));
    lcdOutF2(F("Error"), F("ESP-01 not found"));
    while (true); // Loiter here forever until human resets this device
  }
  /*** End Detect and Start ESP-01 ***/


  /*** Connect to WiFi network ***/
  checkMemState();
  lcdOutF1(F("WiFi CXN INIT"), String(gWiFiSsid));
  int wifiStatus = WL_IDLE_STATUS;
  while (wifiStatus != WL_CONNECTED) {     // Repeat till connected to WiFi network
    serialOutF1(F("Attempting to connect to WPA SSID"), gWiFiSsid);

    wifiStatus = WiFi.begin(gWiFiSsid, gWiFiPass);  // Connect to WPA/WPA2 network

    if(wifiStatus != WL_CONNECTED){
      lcdOutF2(F("WiFi"), F("CXN failed"));
      delay(DELAY_ERR);
    }
  }
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
  wdtResetDisable();
  serialOutF(F("*** Loop Start ***"));
  /*** End Startup  ***/


  /*** Connect to server ***/
  checkMemState();
  wdtResetIfWaitShort();
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

  wdtResetDisable();
  serialOutF(F("Connected to Local Server"));
  lcdOutF2(F("Local Server"), F("CXN Success"));
  /*** End Connect to server ***/


  /*** Data Integrity check ***/
  checkMemState();
  wdtResetIfWaitShort();

  WebClient.setTimeout(HTTP_TIMEOUT);
  bool ok = WebClient.find(CHR_CONTENT_START);
  if (!ok) {
    serialOutF(F("No data or invalid data received from Local Server. Retrying..."));
    lcdOutF2(F("Local Server"), F("RX data null"));
    windUpOnError();
    return;
  }

  wdtResetDisable();
  /*** End Data Integrity check ***/


  /*** Start Data parse ***/
  checkMemState();
  wdtResetIfWaitShort();

  char response[MAX_CONTENT_SIZE];
  size_t length = WebClient.readBytes(response, sizeof(response));
  response[length] = 0;
  serialOutF1(F("Server Response"), response);

  const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(2);  // the root object has 2 elements
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer; // Allocate a temporary memory pool on the stack
  JsonObject& root = jsonBuffer.parseObject(response);

  if (root.success()){
    char *line0 = root["0"];
    char *line1 = root["1"];
    lcdOut(line0, line1);
    serialOutMpEvent(line0, line1);
  }else{
    serialOutF(F("Could not split Local Server data. Retrying..."));
    lcdOutF2(F("Local Server"), F("RX data invalid"));
    windUpOnError();
    return;
  }

  wdtResetDisable();
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
  wdtResetDisable();
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
void serialOutMpEvent(char *line0, char *line1){
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
  Serial.print(F(": "));
  Serial.println(variable);
}

//  Write output to LCD. Low memory fix. No string is literal
//  type T: New catch-all Type
void lcdOut(char *line1, char *line2){
  LCD.clear();
  delay(20);
  LCD.setCursor(0, 0);
  LCD.print(line1);
  delay(20);
  LCD.setCursor(0, 1);
  LCD.print(line2);
}

//  Write output to LCD. Low memory fix. One string is literal
//  type T: New catch-all Type
template<typename T> void lcdOutF1(const __FlashStringHelper *line1, T line2){
  LCD.clear();
  delay(20);
  LCD.setCursor(0, 0);
  LCD.print(line1);
  delay(20);
  LCD.setCursor(0, 1);
  LCD.print(line2);
}

//  Write output to LCD. Low memory fix. Both strings are literal
void lcdOutF2(const __FlashStringHelper *line1, const __FlashStringHelper *line2){
  LCD.clear();
  delay(20);
  LCD.setCursor(0, 0);
  LCD.print(line1);
  delay(20);
  LCD.setCursor(0, 1);
  LCD.print(line2);
}
/*** End IO & Formatting  ***/


/*** WDT & Reset ***/
//  FRAM = free RAM
//  CRUN = complete executions of loop. 0 index.
void checkMemState(){
  int freeRam = getFreeRam();
  String strFreeRam = String(freeRam);
  String strRunCount = String(gRunCount);

  //  Low memory. Reset Arduino
  if(freeRam < FREE_RAM_THRESHOLD){
    serialOutF1(F("MEM CHK: RST RAM LOW. FRAM CRUN"), strFreeRam + " " + strRunCount);
    lcdOutF1(F("RST: FMEM; RUN"), strFreeRam + "; " + strRunCount);
    wdtResetNow();  // Restart Arduino
  }else{
    serialOutF1(F("MEM CHK: FRAM CRUN"), strFreeRam + " " + strRunCount);
  }
}

void wdtResetNow() {
  wdt_enable(WDTO_2S);  //  Init WDT reset after 2s
  while(1) {};  // Loiter here till timer expires
}

void wdtResetIfWaitShort(){
  wdt_enable(WDTO_4S);
}

void wdtResetIfWaitLong(){
  wdt_enable(WDTO_8S);
}

void wdtResetDisable(){
  wdt_reset();
  wdt_disable();
}

int getFreeRam(){
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
/*** End WDT & Reset  ***/
