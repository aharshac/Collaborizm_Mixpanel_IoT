const moment = require('moment');

const config = {};

// https://a6dc44e0b9cec5a2feeacc8eb2a5e164@mixpanel.com/api/2.0/events?from_date=2016-11-11&to_date=2016-11-22&unit=minute&event=compile
// https://a6dc44e0b9cec5a2feeacc8eb2a5e164@data.mixpanel.com/api/2.0/export?from_date=2016-11-11&to_date=2016-11-21&event=["compile"]&where=properties["timestamp"]>1479795414


// From Mixpanel project settings.
config.mp_auth_token = "paste token here";

// From Mixpanel project settings.
config.mp_api_secret = "paste token here";

// From Mongolab (https://mlab.com/)
config.mlab_db_url = "paste url here"

// Copy to Clipboard for debug.
config.copy = function(str){
	var ncp = require("copy-paste");
	ncp.copy(str, function () {});
};

config.getLogTime = function(){
	return moment().format("DD-MM-YYYY HH:mm:ss")
}

module.exports = config;
