# Collaborizm Mixpanel IoT Project
A project on [Collaborizm](https://www.collaborizm.com/) to download and use Mixpanel Event data on IoT devices.

## 1. Supported devices
* Arduino UNO: Client
* Arduino Mega 2560: Client (ongoing)
* NodeMCU: Client (ongoing)
* Raspberry Pi: Server (ongoing)

Until Raspberry Pi Server is complete, run Node_Server on PC, with the clients on the same WiFi as the PC.

## 2. Node.js Server (PC or RPi)
Downloads data from Mixpanel server and saves it in the Local Database at regular intervals.

### 2.1 Mixpanel Event Schema
``Node_Server\mixpanel.js``  
Data sent to Mixpanel by Collaborizm server (and this node, for test purposes).
```
mp.track("Reply", {
  city: "Mangalore",
  country: 'India',
  date: moment(Date.now()).format(),	// Human readable date
  timestamp: moment(Date.now()).valueOf()	// unix timestamp in ms
});
```

### 2.2 Local Database Schema
``Node_Server\db.js``  
How our Local Database stores data imported from Mixpanel.
```
db.eventSchema = new Schema({
  name: {type: String, required: true}, // event name
  city: {type: String, required: true},
  country: {type: String, required: true},
  date: {type: Date, default: moment(Date.now()).format()}, // human readable date, ISO 8601
  timestamp: {type: Number, default: moment(Date.now()).valueOf()}	// unix timestamp in ms
});
```

### 2.3 API Keys
``Node_Server\config.js``   
Sign up and use API keys for Mixpanel and MongoDB (if not using local DB).

### 2.4 Local Server
``Node_Server\webserver.js``   
Configure port and other web server related stuff.

#### 2.4.1 REST API
**2.4.1.1**  ``/``  
Status check.

**2.4.1.2**  ``/events?``
JSON array of all events judged by following parameters.
```
Parameters:

name: Single or comma separated event names
cols: Single or comma separated projection. Default: "name, city, country, date, timestamp"
from: unix timestamp (ms) start
to: unix timestamp (ms) end
last: latest event only (single), can be combined with above params. Supply with dummy value.
```

**2.4.1.3**  ``/events/arduino``
Formatted string for Arduino. Hack to get around low memory problems.
```
No spaces in between.
Output: Char(30)\n <line 1 text>\n <line 2 text>\n Char(31)    
```

**2.4.1.4**  ``/time``
Server time. Used to set time for RTCs in client devices.
```
Output: Human readable date and time. Not ISO 8601 time.
```

## 3. Arduino UNO Client
Sends GET request to Local Server and displays event data on the LCD at regular intervals. Uses WDT to reset when memory decreases.

**Shields:**   
ESP-01 WiFi Module and 16x2 LCD

**Libraries:**  
Download from ``Sketch > Include Library > Manage Libraries``   
StringSplitter and WiFiEsp.

**Circuit diagram:**  
Will be added soon.

To be continued...
