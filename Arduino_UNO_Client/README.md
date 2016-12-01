# **Collaborizm Mixpanel IoT Project**
A project on **[Collaborizm](https://www.collaborizm.com/)** to download and use Mixpanel Event data on IoT devices.   

Read more at the repo root **[README](https://github.com/aharshac/Collaborizm_Mixpanel_IoT/blob/master/README.md).**

&nbsp;

## **Arduino UNO Client**
Sends GET request to Local Server and displays event data on the LCD at regular intervals. Uses WDT to reset when memory decreases.

### Shields:
16x2 LCD  
ESP-01 WiFi Module  

### Libraries:
ArduinoJson   
~~StringSplitter~~   
WiFiEsp

&nbsp;

## **Circuit diagram**
Designed at **[circuits.io](https://circuits.io/circuits/3389638-arduino-esp-01-wifi-lcd)**   

![Circuit Diagram](https://github.com/aharshac/Collaborizm_Mixpanel_IoT/raw/master/Arduino_UNO_Client/Circuit%20Diagram.png "Circuit Diagram")

&nbsp;

## **Pin Configuration**  
### 1. LCD  

| Pin No. | Name         | Connects to                              |
|---------|--------------|------------------------------------------|
| 1       | VSS          | Common GND                               |
| 2       | VDD          | Arduino +5V                              |
| 3       | Contrast     | 2K ohm resistor to Common GND            |
| 4       | RS           | Arduino D15 (A1)                         |
| 5       | R/W          | Common GND                               |
| 6       | Enable       | Arduino D14 (A0)                         |
| 7       | DB0          | Arduino D4                               |
| 8       | DB1          | Arduino D5                               |
| 9       | DB2          | Arduino D6                               |
| 10      | DB3          | Arduino D7                               |
| 11      | DB4          | Arduino D8                               |
| 12      | DB5          | Arduino D9                               |
| 13      | DB6          | Arduino D10                              |
| 14      | DB7          | Arduino D11                              |
| 15      | LED+         | 200 ohm resistor to Arduino +5V          |
| 16      | LED-         | Common GND                               |


### 2. ESP-01 (ESP8266)  
| Name         | Connects to                              |
|--------------|------------------------------------------|
| TXD          | Arduino D3                               |
| CH_PD        | Power Supply +3.3V                       |
| RST          | Arduino Reset                            |
| VCC          | Power Supply +3.3V                       |
| RXD          | Arduino D2                               |
| GPIO0        | {None}                                   |
| GPIO2        | {None}                                   |
| GND          | Common GND                               |

&nbsp;

## **Build Instructions**
1. Build the circuit.
2. Download libraries from ``Sketch > Include Library > Manage Libraries``
3. Change WiFi SSID and password.
4. Change Local Server IP.
5. Select ``Arduino UNO`` from ``Tools > Board``
6. Select **COM Port** from ``Tools > Port``
7. Compile and Run.
8. Open Serial Monitor to read debug logs.
