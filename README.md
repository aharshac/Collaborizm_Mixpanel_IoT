# **Collaborizm Mixpanel IoT Project**
A project on **[Collaborizm](https://www.collaborizm.com/)** to download and use Mixpanel Event data on IoT devices.

&nbsp;

## **Supported devices**
* Arduino UNO: Client.  **[README](https://github.com/aharshac/Collaborizm_Mixpanel_IoT/blob/master/Arduino_UNO_Client/README.md)**
* Arduino Mega 2560: Client (ongoing)
* NodeMCU: Client (ongoing)
* PC or Raspberry Pi: Server (ongoing)  **[README](https://github.com/aharshac/Collaborizm_Mixpanel_IoT/blob/master/Node_Server/README.md)**

Until Server is ported to Raspberry Pi, run Node_Server package on PC, with the clients on the same WiFi as the PC.

&nbsp;

## **Network Nodes**
### 1. Node.js Server (PC or RPi)
Downloads data from Mixpanel server and saves it in the Local Database at regular intervals.

### 2. Arduino UNO Client
Sends GET request to Local Server and displays event data on the LCD at regular intervals. Uses WDT to reset when memory decreases.

<img src="https://github.com/aharshac/Collaborizm_Mixpanel_IoT/raw/master/Arduino_UNO_Client/Circuit%20Diagram.png" width="437" height="246" />

&nbsp;

## **Important Stuff**
### 1. **README** is meant to be read.

### 2. **Mixpanel Event Schema**
File: ``Node_Server\mixpanel.js``  
Data sent to Mixpanel by Collaborizm server (and this node, for test purposes).
```
mp.track("Reply", {
  city: "Mangalore",
  country: 'India',
  date: moment(Date.now()).format(),	// Human readable date
  timestamp: moment(Date.now()).valueOf()	// unix timestamp in ms
});
```

### 3. **Local Database Schema**
File: ``Node_Server\db.js``  
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

### 4. **REST API**
#### 1. ``http://server_ip:8970``
REST API Server. Default port is ``8970``, subject to change.   
Watch ``node console`` to catch it.

#### 2.  ``/``  
Status check.

#### 3.  ``/events?``   
JSON array of all events judged by following parameters.   
**Parameters:**
```
name: Single or comma separated event names
cols: Single or comma separated projection. Default: "name, city, country, date, timestamp"
from: unix timestamp (ms) start
to: unix timestamp (ms) end
last: latest event only (single), can be combined with above params. Supply with dummy value.
```   

#### 4.  ``/events/arduino``    
JSON Object containing line data. Directly displayed on Arduino without processing.  
_Deprecated:_ ~~Formatted string for Arduino. Hack to get around low memory problems.~~   
**Output:**  The character (ASCII 30) prepended to the JSON string is used to determine the start of content.  
```
{"0": "<line 0 text>", "1": "<line 1 text>"}    
```

#### 5.  ``/time``    
Server time. Used to set time for RTCs in client devices.   
**Output:**   
```
Human readable date and time. Not ISO 8601 time.
```
