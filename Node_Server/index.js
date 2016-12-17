/*
  Package: Node_Server
  Description: Node app and Local Webserver for other IoT devices. Made for Collaborizm Mixpanel IoT project.
  Version: 1.0
  Author: Harsha Alva

  This Node package downloads Mixpanel export data and stores it in a local database.
  It also provides a Rest API to access this data locally.

  This Node package is future ready, i.e., new client devices can be added in the future.

  For more details see: https://github.com/aharshac/Collaborizm_Mixpanel_IoT
*/

const cron = require('node-cron');
const moment = require('moment');

const db = require('./db');
const config = require('./config');
const mixpanel = require('./mixpanel');
const persist = require('./persist');
const rpi = require('./rpi');
const server = require('./webserver');

// Connect to database
db.Connect();

rpi.init();

// Start Express Server
server.start(function(ipAddress, port){
	mixpanel.trackStart();	// Track test event on Mixpanel

	if (rpi.LCD_is20x4)
		rpi.stdOutLcd("IP: " + ipAddress, "Port: " + port);
	else
		rpi.stdOutLcd("Mixpanel IoT", ipAddress);
});

//	Schedule cron jobs every 2 mins to import Mixpanel export data
cron.schedule('*/1 * * * *', function(){
	mixpanel.getMpData();

	displayLatestEventOnPiLcd();
});


// If RPi, display latest downloaded event
function displayLatestEventOnPiLcd(){
	if(!rpi.isRpi)
		return;

	mixpanel.getLatestStoredEventForLcd(function(err, event){
		if(err){
			console.log("RPi LCD data error @ " + config.getLogTime() + ": " + err);
		}else{

			var name = event.name;
			var city = event.city;
			var country = event.country;
			var timestamp = event.timestamp;

			var date_format = (rpi.LCD_is20x4 ? "DD/MM HH:mm" : "HH:mm");
			var date = "";


			if(moment(timestamp).isValid()){
				date = moment(timestamp).format(date_format);
			}

			var line0 = name + ", " + date;
			var line1 = city + ", " + country;

			rpi.stdOutLcd(line0, line1);

			console.log("RPi LCD data @ " + config.getLogTime() + ": " + JSON.stringify({"0": line0, "1": line1}));
		}
	});
}
