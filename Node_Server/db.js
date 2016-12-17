const moment = require('moment');
const mongoose = require('mongoose');

const config = require('./config');

var Schema = mongoose.Schema;

const db = {};



//	Use if MongoDB is installed locally.
//db.url_db = 'mongodb://localhost/arduino-mixpanel';

//	Use if using MongoLab DB in cloud
db.url_db = config.mlab_db_url;



// 	deprecation warning fix
mongoose.Promise = global.Promise;


//	Open connection to database
db.Connect = function(){
	mongoose.connect(db.url_db);
}


mongoose.connection.on('connected', function () {
	console.log('DB connected: ' + db.url_db);
});

mongoose.connection.on('error',function (err) {
	console.log('DB connection error: ' + err);
});

mongoose.connection.on('disconnected', function () {
	console.log('DB disconnected');
});


//	DB schema for event
db.eventSchema = new Schema({
	name: {type: String, required: true},
	city: {type: String, required: true},
	country: {type: String, required: true},
	date: {type: Date, default: moment(Date.now()).format()},
	timestamp: {type: Number, default: moment(Date.now()).valueOf()}	// unix timestamp in ms
});

// Event object Model
db.Event = mongoose.model('Event', db.eventSchema);

module.exports = db;
