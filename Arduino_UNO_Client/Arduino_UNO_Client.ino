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
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include "WiFiEsp.h"
#include "StringSplitter.h"


const int FREE_RAM_THRESHOLD = 300; //  Reset board if RAM is below this amount

//  Intervals
const unsigned long DELAY_POLL_INTERVAL = 60000L; // Time between updates (ms)
const unsigned long DELAY_WAIT = 1000L; // Time to wait to show device status messages
const unsigned long DELAY_ERR = 2000L; // Time to wait after errors, before continuing
const unsigned long DELAY_ERR_LONG = 10000L; // Time to wait after bigger errors, before continuing

//  Data format helpers
const char CHR_CONTENT_START = char(30);    //  Server output end. ASCII code for record separator
const char CHR_CONTENT_END = char(31);  //  Server output start.  ASCII code for unit separator

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
//const int pinLed = 13;  //  On board LED


//  WiFi config
#error "Change ASAP"
char gWiFiSsid[] = "";  // Wifi Network SSID (name)
#error "Change ASAP"
char gWiFiPass[] = ""; // Wifi password

//  Server details
char gServerName[] = "192.168.1.2";
const int gServerPort = 8970;
String gServerRestPath = "/events/arduino";  // Arduino formatted OP

int gRunCount = 0;   // Run counter


/***  Expansions and shields ***/
SoftwareSerial ESP8266(pinEspTx, pinEspRx);   // ESP8266 Wifi Module
//LiquidCrystal LCD(pinLcdRs, pinLcdE, pinLcdD4, pinLcdD5, pinLcdD6, pinLcdD7); // Init serial
LiquidCrystal LCD(pinLcdRs, pinLcdE, pinLcdD0, pinLcdD1, pinLcdD2, pinLcdD3, pinLcdD4, pinLcdD5, pinLcdD6, pinLcdD7); // Init parallel

void setup() {
  wdtResetDisable();  // Disable WDT ASAP

  Serial.begin(9600); // Initialize UART serial for debugging
  checkMemState();

  //setLed((HIGH);

  ESP8266.begin(9600);  // Initialize Software UART serial for ESP module
  LCD.begin(16,2);  //  Initialize 16x2 LCD
  //pinMode(pinLed, OUTPUT);  //  Set LED pin as Output

  lcdOutF2(F("Collaborizm"), F("Arduino-Mixpanel"));


  WiFi.init(&ESP8266); // initialize ESP module
  checkMemState();

  if (WiFi.status() == WL_NO_SHIELD) {  // Check for the presence of the ESP
    serialOutF(F("ESP WiFi shield not present"));
    lcdOutF2(F("Error"), F("ESP-01 not found"));
    //setLed(LOW);
    while (true); // Loiter here forever until human resets this device
  }

  //setLed(HIGH);

  checkMemState();
  lcdOutF1(F("WiFi CXN INIT"), String(gWiFiSsid));
  int wifiStatus = WL_IDLE_STATUS;
  while (wifiStatus != WL_CONNECTED) {     // Repeat till connected to WiFi network
    serialOutF1(F("Attempting to connect to WPA SSID"), gWiFiSsid);

    wifiStatus = WiFi.begin(gWiFiSsid, gWiFiPass);  // Connect to WPA/WPA2 network

    if(wifiStatus != WL_CONNECTED){
      lcdOutF2(F("WiFi"), F("CXN failed"));
      //setLed(LOW);
      delay(DELAY_ERR);
    }
  }

  //setLed(HIGH);

  checkMemState();

  serialOutF(F("Connected to wifi"));
  lcdOutF1(F("WiFi IP"), ipToString(WiFi.localIP()));
  outWiFiStats();

  //setLed(LOW);

  delay(DELAY_WAIT);
}

void loop(){
  fetchAndDisplayData();
}

void fetchAndDisplayData() {
  wdtResetDisable();

  //setLed(HIGH);
  serialOutF(F("*** Loop Start ***"));

  WiFiEspClient webClient;
  String response = "";

  checkMemState();
  wdtResetIfWaitShort();
  // Send GET REQUEST
  if (webClient.connect(gServerName, gServerPort)) {
    webClient.println("GET " + gServerRestPath + " HTTP/1.1");
    webClient.println("Host: " + String(gServerRestPath));
    webClient.println("Connection: close");
    webClient.println();
  } else {
    //setLed(LOW);
    serialOutF(F("Connection failed"));
    lcdOutF2(F("Local Server"), F("CXN Failure"));
    wdtResetDisable();
    delay(DELAY_ERR_LONG);
    return;
  }
  wdtResetDisable();

  serialOutF(F("Connected to Local Server"));
  lcdOutF2(F("Local Server"), F("CXN Success"));

  checkMemState();
  wdtResetIfWaitShort();
  //  Wait till server outputs data
  long waitStart = millis();
  while(!webClient.available()){
    if((millis() - DELAY_WAIT) > waitStart){  // Timeout
      serialOutF(F("No data received from Local Server. Retrying..."));
      lcdOutF2(F("Local Server"), F("RX data null"));
      //setLed(LOW);
      //wdtResetDisable();
      delay(DELAY_ERR_LONG);
      return;
    }
    delay(200);
  }
  wdtResetDisable();


  checkMemState();
  wdtResetIfWaitShort();
  //  Read and check data format
  if(webClient.available()) {
    response = webClient.readStringUntil(CHR_CONTENT_END);  // Read till CHR_CONTENT_END is received
    serialOutF1(F("Local server output"), "\n" + response + "\n");
    if(response.indexOf(CHR_CONTENT_START) == -1){  //  CHR_CONTENT_START not present in response
      serialOutF(F("Invalid data received from Local Server. Retrying..."));
      lcdOutF2(F("Local Server"), F("RX data invalid"));
      //setLed(LOW);
      //wdtResetDisable();
      delay(DELAY_ERR_LONG);
      return;
    }
  }else{
    serialOutF(F("Out of memory. Restarting......"));
    lcdOutF2(F("Out of memory"), F("Restarting......"));
    delay(DELAY_WAIT);
  }
  wdtResetDisable();


  checkMemState();
  wdtResetIfWaitShort();
  //  Parse data
  int posContentStart = response.lastIndexOf(CHR_CONTENT_START);
  if(posContentStart > 0){
    response = response.substring(posContentStart + 1);  // First char pos + newline char
    response.replace("\r", ""); //  Remove all CR

    StringSplitter *split = new StringSplitter(response, '\n', 2);
    int lineCount = split->getItemCount();

    //  Validate if lines could be split
    if(lineCount == 0){
      serialOutF(F("Could not split Local Server data. Retrying..."));
      lcdOutF2(F("Local Server"), F("RX Data Split ERR"));
      //setLed(LOW);
      //wdtResetDisable();
      delay(DELAY_ERR_LONG);
      return;
    }


    // Display on LCD
    LCD.clear();
    serialOutF(F("LCD Output"));
    for(int i = 0; i < lineCount; i++){
      String line = split->getItemAtIndex(i);
      if(line.length() > 0){
        LCD.setCursor(0, i);
        LCD.print(line);
        delay(20);
        serialOutF1(F("Line"), String(i + 1) + " " + line);
      }
    }
  }
  wdtResetDisable();
  checkMemState();

  webClient.flush(); //  Flush all previous received and transmitted data
  webClient.stop();
  checkMemState();

  serialOutF(F("*** Loop End ***\n"));
  ++gRunCount;  //  Increment run counter

  delay(DELAY_POLL_INTERVAL); // Wait before next update check
}



/*** IO & format ***/

//  Output WiFi Station info to Serial
void outWiFiStats(){
  serialOutF1(F("SSID"), WiFi.SSID());

  IPAddress ip = WiFi.localIP();  //  IP address of ESP8266
  serialOutF1(F("IP Address"), ip);

  long rssi = WiFi.RSSI();  //  Signal strength
  serialOutF1(F("Signal strength (RSSI) (dBm)"), rssi);
}

//  Format IP address
String ipToString(const IPAddress &ip){
  if(!ip)
    return "IP not found";
  else
    return String(ip[0]) + String(".") + String(ip[1]) + String(".") + String(ip[2]) + String(".") + String(ip[3]);
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

/*
void setLed(int val){
  digitalWrite(pinLed, val);
}
*/

/*** WDT & Reset ***/

void checkMemState(){
  int freeRam = getFreeRam();
  String strFreeRam = String(freeRam);
  String strRunCount = String(gRunCount);

  //  Low memory. Reset Arduino
  if(freeRam < FREE_RAM_THRESHOLD){
    serialOutF1(F("MEM CHK: Restarting Arduino due to RAM shortage. FRAM (B); FULL RUNS"), strFreeRam + "; " + strRunCount);
    lcdOutF1(F("RST: FMEM; RUN"), strFreeRam + "; " + strRunCount);
    wdtResetNow();  // Restart Arduino
  }else{
    serialOutF1(F("MEM CHK: Free RAM (bytes); Full Run Count"), strFreeRam + "; " + strRunCount);
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
