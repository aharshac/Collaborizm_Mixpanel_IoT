const storage = require('node-persist');

const config = require('./config');

const persist = {};

persist._dir = ".persist";
persist._prop_last_update_time = "update";

persist.getLastUpdateTime = function(){
	try {
		//var contents = fs.readFileSync(persist._file_name, 'utf8');
		storage.initSync({dir: persist._dir});
		var time = storage.getItemSync(persist._prop_last_update_time);
		if (typeof time == "number" && !isNaN(time)){
			return time;
		}else{
			return 1;
		}
	}catch (err) {
		console.log("Persist getLastUpdateTime error: " + err);
		return 2;
	}
};

persist.setLastUpdateTime = function(time){
	try {
		if (typeof time == "number" && !isNaN(time)){
			//fs.writeFileSync(persist._file_name, time);
			storage.initSync({dir: persist._dir});
			storage.setItemSync(persist._prop_last_update_time, time);
		}
	}catch (err) {
		console.log("Persist setLastUpdateTime error: " + err);
	}
};

persist.clear = function(callback){
	try {
		storage.initSync({dir: persist._dir});
		storage.clearSync();
		callback;
	}catch (err) {
		console.log("Persist clear error: " + err);
	}
};


module.exports = persist;