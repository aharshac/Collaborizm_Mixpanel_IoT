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

const db = require('./db');
const config = require('./config');
const mixpanel = require('./mixpanel');
const persist = require('./persist');
const server = require('./webserver');

// Connect to database
db.Connect();

// Start Express Server
server.start(function(){
	mixpanel.trackStart();	// Track test event on Mixpanel
});

//	Schedule cron jobs every 2 mins to import Mixpanel export data
cron.schedule('*/2 * * * *', function(){
	mixpanel.getMpData(); 
}); 



