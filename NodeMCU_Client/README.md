# **Collaborizm Mixpanel IoT Project**
A project on **[Collaborizm](https://www.collaborizm.com/)** to download and use Mixpanel Event data on IoT devices.   

Read more at the repo root **[README](https://github.com/aharshac/Collaborizm_Mixpanel_IoT/blob/master/README.md).**

&nbsp;

## **NodeMCU DEVKIT V3 (Chinese clone) Client**    
Read more at [NodeMCU Wikipedia Page](https://en.wikipedia.org/wiki/NodeMCU)  

Sends GET request to Local Server and displays event data on the LCD at regular intervals.

The Chinese V3 clones are a bit larger than the regular V2 board. But one reserved pin is connected to USB +5V and another to GND in these clones.  
Read comparison [here](http://frightanic.com/iot/comparison-of-esp8266-nodemcu-development-boards/)

### Shields:
16x2 LCD I2C LCD (Or I2C LCD Adapter Module)

### Libraries:
ArduinoJson  
LiquidCrystal_I2C

&nbsp;

## **Circuit diagram**
Will be added at a later date as no Circuit Design software supports NodeMCU V2 or V3 at this date.

&nbsp;

## **Pin Configuration**  
Very simple as we are using LCD I2C Adapter board. Hence, only 4 female-female jumpers are required.

### NodeMCU

| Name         | Connects to                              |
|--------------|------------------------------------------|
| GND (G)      | I2C LCD GND                              |
| VUSB (V)     | I2C LCD VCC                              |
| D1           | I2C LCD SCL                              |
| D2           | I2C LCD SDA                              |

&nbsp;

## **Install ESP8266 Platform in Arduino IDE**
Starting with 1.6.4, Arduino allows installation of third-party platform packages using Boards Manager. We have packages available for Windows, Mac OS, and Linux (32 and 64 bit).

1. Install Arduino 1.6.8 from the [Arduino website](http://www.arduino.cc/en/main/software).
2. Start Arduino IDE.
3. Open Preferences window by ```File > Preferences```
4. Enter ```http://arduino.esp8266.com/stable/package_esp8266com_index.json``` into **Additional Board Manager URLs** field. You can add multiple URLs, separating them with commas.
5. Open **Boards Manager** from ```Tools > Board > Boards Manager``` and install **ESP8266** platform.

&nbsp;

## **Build Instructions**
1. Connect pin jumpers to pins.
2. Download libraries from ``Sketch > Include Library > Manage Libraries``
3. Change WiFi SSID and password.
4. Change Local Server IP.
5. Select ``NodeMCU 1.0 (ESP-12E Module)`` from ``Tools > Board``
6. Select **COM Port** from ``Tools > Port``
7. Compile and Run.
8. Open Serial Monitor to read debug logs.
