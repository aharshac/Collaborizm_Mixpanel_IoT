const express = require('express');
const mongoose = require('mongoose');
const moment = require('moment');
const lazy = require('lazy.js');
const ip = require('ip');

const config = require('./config');
const db = require('./db');
const mixpanel = require('./mixpanel');
const persist = require('./persist');

/* REST END POINT */

const server = {};

server.app = express();

//	Start web server
server.start = function(callback){
	server.server = server.app.listen(8970, function() {
		console.log('Mixpanel IoT Node Server started @ ' + ip.address() + ':8970');
		callback();
	});
};


//	Status check
server.app.get('/', (req, res) => {
	res.send('Mixpanel IoT Node Server is working @ ' + ip.address());
});


// API: /events?
/*	GET Parameters
name: Single or comma separated event names
cols: Single or comma separated projection. Default: "name, city, country, date, timestamp"
from: unix timestamp (ms) start
to: unix timestamp (ms) end
last: latest event only (single), can be combined with above params. supply with dummy value.

*/

server.app.get('/events', (req, res) => {
	var url = req.protocol + '://' + req.get('host') + req.originalUrl;

	var cols = req.query.cols;
	var name = req.query.name;
	var from = req.query.from;
	var to = req.query.to;
	var latest = req.query.last;

	var projection = "";
	var selection = [];
	var rows = 0;

	if (cols){
		cols = unescape(cols);

		if(cols.indexOf(",")){
			lazy(cols)
			.split(',')
			.each(function(col){
				projection += col + " ";
			});
		}else{
			projection = cols;
		}
		projection = projection.trim();
	}
	//console.log(projection);

	if(!projection)
			projection = 'name city country date timestamp';

	if (name){
		name = unescape(name);

		if(name.indexOf(",")){
			lazy(name)
			.split(',')
			.each(function(event){
				selection.push(event);
			});
		}else{
			selection.push(event);
		}
	}

	var find = db.Event.find({}, projection);

	if(selection.length > 0){
		find.where('name').in(selection);
	}

	if(from){
		if(moment(from).isValid())
			find.where('timestamp').gt(moment(from).valueOf());
	}

	if(to){
		if(moment(to).isValid())
			find.where('timestamp').lt(moment(to).valueOf());
	}

	if(latest){
		find.sort({'timestamp': -1});
		find.limit(1);
	}else{
		find.sort({'timestamp': 1});
	}


	find.exec(function(err, events) {
		if (err){
			res.send('{"Error":"' + err + '"}');
			console.log("REST API Query Error @ " + config.getLogTime() + ": " + err);
		}else{
			rows = events.length;

			if(latest){
				if(rows == 1){
					var first = events[0];
					res.send(JSON.stringify(first));
				}else{
					res.send("");
				}
			}else{
				res.send(JSON.stringify(events));
			}
			console.log("REST API Query @ " + config.getLogTime() + ": Events (Rows = " +  rows + ") " + url);
		}

	});
});


// API: /events/arduino
/*
	Arduino/Esp8266 lacks juice. Unless we can dope it with viagra, json parsing takes time and resources.
	Configure required LCD output here itself.
	Output: Char(30)\n <line 1 text>\n <line 2 text>\n Char(31)  
	Deprecated: Output each line to be displayed on arduino as @OP::<line 1 content goes here>;<line 2 content goes here>\n.
*/

server.app.get('/events/arduino', (req, res) => {
	var projection = 'name city country date timestamp';	// columns. col timestamp is required for sorting
	var selection = mixpanel.required_events;			//	event name filter
	var rows = 0;

	var find = db.Event.find({}, projection);

	if(selection.length > 0){
		find.where('name').in(selection);
	}

	find.sort({'timestamp': -1});
	find.limit(1);


	find.exec(function(err, events) {
		if (err){
			res.send('{"Error":"' + err + '"}');
			console.log("REST API Query Error @ " + config.getLogTime() + ": " + err);
		}else{
			if(events.length == 1){
				var op = "";

				var event = events[0];

				var name = event.name;
				var city = event.city;
				var country = event.country;
				var timestamp = event.timestamp;

				// fucking timezone problem. date pushed to mixpanel as ISO 8601 with added timezone offset
				// fixed problem by using timestamp instead of date
				// subtract 5.5 hours until fixed
				if(moment(timestamp).isValid()){
					date = moment(timestamp).format("HH:mm");			// need only the time, for now (1602 lcd)
					//date = moment(date).subtract(5.5, 'hours').format("H:mm");			// DONE:: TODO: fix mixpanel ISO 8601 string spec
				}

				op = String.fromCharCode(30) + "\n";	//Content start: ASCII code for record separator
				op += name + ", " + date + "\n";
				op += city + ", " + country + "\n";
				op += String.fromCharCode(31);	// ASCII code for unit separator
				res.send(op);
			}else{
				res.send("");
			}
			console.log("REST API Query @ " + config.getLogTime() + ": Events/Arduino\n" + op + "\n");
		}
	});
});


// Not ISO 8601 time. Human readable.
server.app.get('/time', (req, res) => {
	var now = moment().format();
	res.send(now);
	console.log("REST API Query @ " + moment().format("DD-MM-YYYY HH:mm:ss") + ": Server time " + now);
});

// Clear all
server.app.get('/clear', (req, res) => {
	console.log('Clearing data');

	db.Event.remove(function(err, removed) {
		if(err)
			console.log("DB clear error " + err);
		else
			console.log("DB cleared " + removed);
	});

	persist.clear(function(){
		console.log("Persist cleared")
	});

	res.send("REST API Query @ " + moment().format("DD-MM-YYYY HH:mm:ss") + ": Cleared data");
});


server.app.get('/stop', (req, res) => {
	//res.redirect('/');

	res.send('Stopping server.....');
	console.log("REST API Query @ " + config.getLogTime() + ": Stopping server.....");

	setTimeout(function () {
		server.server.close();

		var readline = require('readline');
		var rl = readline.createInterface({
			input: process.stdin,
			output: process.stdout
		});
		rl.write(null, {ctrl: true, name: 'c'});
		rl.write(null, {ctrl: false, name: 'y'});
		process.exit();
	}, 1000);
});





module.exports = server;
