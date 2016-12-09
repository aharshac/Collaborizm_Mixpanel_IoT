const mongoose = require('mongoose');
const moment = require('moment');
const request = require('request');
const lazy = require('lazy.js');
const mp_prototype = require("mixpanel");

const db = require('./db');
const config = require('./config');
const persist = require('./persist');

const mixpanel = {};

mixpanel.mp = mp_prototype.init(config.mp_auth_token);

// Events that we want to track and/or download from Mixpanel.
mixpanel.required_events = ["Reply"];


//	TODO: Remove when using in Collaborizm. Example: Track each compile on Mixpanel
mixpanel.trackStart = function(){
	mixpanel.mp.track(mixpanel.required_events[0], {
		city: "Mangalore",
		country: 'India',
		date: moment(Date.now()).format(),	// Human readable date
		timestamp: moment(Date.now()).valueOf()	// unix timestamp in ms
	});
	console.log("Start event tracked in Mixpanel");
};


// 	Get Mixpanel API end point url
// 	Mixpanel date format: YYYY-MM-DD
//	events: Array of strings ["1", "a", "b"] etc; 	deprecated: formatted string '["compile"]'
//	propTimestamp: name of property used to store unix timestamp (ms) in Mixpanel
//	timestamp: unix timestamp. Uniqueness check. Only events with timestamps above this will be downloaded.
mixpanel.getMpRequestUrl = function(events, propTimestamp, timestamp){
	var selection = "[";
	if (events){
		lazy(events)
			.each(function(event){
				selection += "\"" + event + "\"" + ",";
			});
	}
	selection = selection.replace(/,\s*$/, "");		//
	selection += "]";

	// Import Mixpanel data for yesterday and today.
	var today = moment();	// Using IST in mixpanel project settings
	//var today = moment().subtract(13.5, 'hours'); // fix for India as Mixpanel uses US Pacific time
	var yesterday = moment().subtract(1, 'day');

	var url = "https://" + config.mp_api_secret + "@data.mixpanel.com/api/2.0/export?from_date=" + yesterday.format('YYYY-MM-DD') + "&to_date=" + today.format('YYYY-MM-DD');

	if(events)
		url += "&event=" + encodeURIComponent(selection);
		//url += "&event=" + selection;

	if(timestamp > 0)
		url += '&where=' + encodeURIComponent('properties["' + propTimestamp + '"]>') + timestamp;

	return url;
};


// 	Get export data from Mixpanel
// 	latest_only: imports only last tracked event in mixpanel, if true
mixpanel.getMpData = function(latest_only){
	console.log('*** Importer Cron @ ' + config.getLogTime());

	var latest_only = typeof latest_only  !== 'undefined' ?  latest_only  : false;

	var lastUpdate = persist.getLastUpdateTime();
	if(lastUpdate > 1 && moment(lastUpdate).isValid())
		lastUpdate = moment(lastUpdate).subtract(1, 'seconds').valueOf();

	console.log("Last max update time = " + moment(lastUpdate).format());
	var url = mixpanel.getMpRequestUrl(mixpanel.required_events, "timestamp", lastUpdate);
	//console.log(url);

	var mp_max_time = moment().valueOf();

	request(url, function (error, response, body) {
		if (!error && response.statusCode == 200) {
			var forLoop = lazy(body).split('\n');
			//forLoop.take(100)			// Cut off limit

			var size = forLoop.size();

			if(size >= 1)
				size -= 1;	// Last line is empty new line

			if(latest_only)
				forLoop = forLoop.last(1);

			forLoop.each(function(line){
				if(!line)
					return;

				//console.log("Line: " + line);

				var event = JSON.parse(line);
				//config.copy(line);

				var name = event.event;
				var timestamp = event.properties.timestamp;
				var city = event.properties.city;
				var country = event.properties.country;
				var date = event.properties.date;

				if(!name || !timestamp || !city || !country || !date)
					return;

				var newEvent = new db.Event({name: name, city: city, country: country, date: date, timestamp: timestamp});

				newEvent.save(function(err) {
					if (err)
						console.log(err);
					//else
						//console.log('Event created!');
				});

				if(timestamp > mp_max_time)
					mp_max_time = timestamp;

				//console.log(name + " " + timestamp + " " + city + " " + country + " " + date);
			});
			persist.setLastUpdateTime(mp_max_time);
			console.log("Next max update time = " + moment(mp_max_time).format());
			console.log('Imported ' + size + ' rows from Mixpanel to local db.');
		}else{
			console.log("Mixpanel API error: code: " + (response != null ? response.statusCode : "null") + " msg: " + error + " body: " + body);
		}
		console.log('*** Finished Importer Cron @ ' + config.getLogTime());
	});
};


module.exports = mixpanel;
