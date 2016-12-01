# **Collaborizm Mixpanel IoT Project**
A project on **[Collaborizm](https://www.collaborizm.com/)** to download and use Mixpanel Event data on IoT devices.   

Read more at the repo root **[README](https://github.com/aharshac/Collaborizm_Mixpanel_IoT/blob/master/README.md)**.

&nbsp;

## **Node.js Server (PC or RPi)**
Downloads data from Mixpanel server and saves it in the Local Database at regular intervals.

&nbsp;

## **Configuration**
### 1. Mixpanel Event Schema
File: ``mixpanel.js``  
Data sent to Mixpanel by Collaborizm server (and this node, for test purposes).
```
mp.track("Reply", {
  city: "Mangalore",
  country: 'India',
  date: moment(Date.now()).format(),	// Human readable date
  timestamp: moment(Date.now()).valueOf()	// unix timestamp in ms
});
```

### 2. Local Database Schema
File: ``db.js``  
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

### 3. API Keys
File: ``config.js``   
Sign up and use API keys for Mixpanel and MongoDB (if not using local DB).

### 4. Local Server
File: ``webserver.js``   
Configure port and other web server related stuff, including REST API paths.  
Default port is ``8970``, subject to change.

&nbsp;

## **REST API**
### 1. ``http://server_ip:8970``
REST API Server. Default port is ``8970``, subject to change.   
Watch ``node console`` to catch it.

### 2.  ``/``  
Status check.

### 3.  ``/events?``   
JSON array of all events judged by following parameters.   
**Parameters:**
```
name: Single or comma separated event names
cols: Single or comma separated projection. Default: "name, city, country, date, timestamp"
from: unix timestamp (ms) start
to: unix timestamp (ms) end
last: latest event only (single), can be combined with above params. Supply with dummy value.
```   

### 4.  ``/events/arduino``    
JSON Object containing line data. Directly displayed on Arduino without processing.  
_Deprecated:_ ~~Formatted string for Arduino. Hack to get around low memory problems.~~   
**Output:**  The character (ASCII 30) prepended to the JSON string is used to determine the start of content. 
```
{"0": "<line 0 text>", "1": "<line 1 text>"}    
```

### 5.  ``/time``    
Server time. Used to set time for RTCs in client devices.   
**Output:**   
```
Human readable date and time. Not ISO 8601 time.
```

&nbsp;

## **Build Instructions**
1. Install [Node.js](https://nodejs.org/)
2. Get API keys from Mixpanel and MongoLab.
3. Open Command Prompt or Terminal console.
4. ``cd`` to this directory.
5. ``npm install -g nodemon``
6. ``npm install``
7. ``npm run dev``
8. Note down Local IP address displayed in the console.
9. To exit, press ``Ctrl + C`` and `y` in the console. Else, `nodemon` will crash.
