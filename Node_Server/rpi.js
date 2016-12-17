const fs = require('fs');
const os = require('os');
const moment = require('moment');

const config = require('./config');

const rpi = {};

rpi.isRpi = false;	// Check once if RPi

rpi.LCD = null;	// LCD object

rpi.LCD_is20x4 = false;	// True if LCD is 20x4, false if 16x2

//	Use sudo i2cdetect -y 1 to find address
if (rpi.LCD_is20x4)
	rpi.LCD_address = 0x3f;
else
	rpi.LCD_address = 0x27;

rpi.os_release_file = "/etc/os-release";	// RPi Raspbian release info

// Initialize LCD. Install LcdLib if not exists.
rpi.init = function(){
	if(os.platform().toLowerCase() == 'linux' && fs.existsSync(rpi.os_release_file)){
		var data = fs.readFileSync(rpi.os_release_file).toString().toLowerCase();
		rpi.isRpi = (data.indexOf("raspbian") > 0);
	}

	console.log("Raspberry Pi Check : " + rpi.isRpi);

	if (!rpi.isRpi)
		return;

	// Init LCD library

	// Install LCD library if not exists
	var lib_lcdi2c;
	try {
    lib_lcdi2c = require('lcdi2c');
	} catch (ex) {
		lib_lcdi2c = null;
	}

	if(!lib_lcdi2c){
		console.log("LcdLib does not exist. Installing....");
		rpi.installLcdLib();
	}else{
		console.log("LcdLib OK");
	}

	try {
    lib_lcdi2c = require('lcdi2c');
	} catch (ex) {
		lib_lcdi2c = null;
		console.log("Could not initialize LCD library.")
	}


	// Init LCD

	// LCD address
	// Use sudo i2cdetect -y 1 to find address and update
	if (rpi.LCD_is20x4)
		rpi.LCD = new lib_lcdi2c(1, rpi.LCD_address, 20, 4);
	else
		rpi.LCD = new lib_lcdi2c(1, rpi.LCD_address, 16, 2);
}


// Install LcdLib
rpi.installLcdLib = function(){
	var exec = require('child_process').exec,
    child;

	child = exec('npm install lcdi2c',
		function (error, stdout, stderr) {
	    console.log('RPi LcdLib installer stdout: ' + stdout);
	    console.log('RPi LcdLib installer stderr: ' + stderr);
	    if (error !== null) {
	          console.log('RPi LcdLib installer exec error: ' + error);
	    }
	});
}


/*** LCD functions ***/
// Clear screen
rpi.lcd_clear = function(){
	if(rpi.isRpi && rpi.LCD != null){
		rpi.LCD.clear();
		if (rpi.LCD.error)
			console.log("LCD error (lcd_clear) " + rpi.LCD.error);
	}
}


// Print
rpi.lcd_print = function(str){
	if(rpi.isRpi && rpi.LCD != null){
		rpi.LCD.print(str);
		if (rpi.LCD.error)
			console.log("LCD error (lcd_print) " + rpi.LCD.error);
	}
}


// Print line.
// line: >= 1
rpi.lcd_println = function(str, line){
	if(rpi.isRpi && rpi.LCD != null){
		rpi.LCD.println(str, line);
		if (rpi.LCD.error)
			console.log("LCD error (lcd_print) " + rpi.LCD.error);
	}
}


// Set Backlight
rpi.lcd_setBacklight = function(state){
	if(rpi.isRpi && rpi.LCD != null){
		//rpi.LCD.setBacklight(state);
		if (state)
			rpi.LCD.on();
		else
			rpi.LCD.off();

		if (rpi.LCD.error)
			console.log("LCD error (lcd_setBacklight) " + rpi.LCD.error);
	}
}


// Set Cursor
rpi.lcd_setCursor = function(col, row){
	if(rpi.isRpi && rpi.LCD != null){
		rpi.LCD.setCursor(col, row);
		if (rpi.LCD.error)
			console.log("LCD error (lcd_setCursor) " + rpi.LCD.error);
	}
}


// LCD formatted output
rpi.stdOutLcd = function(data1, data2){
	rpi.lcd_clear();
	rpi.lcd_setBacklight(true);

	// Display project title, current time and data (2 rows) for 20x4
	// Display only data (2 rows) for 16x2

	if (rpi.LCD_is20x4){
		rpi.lcd_setCursor(0,0);
		rpi.lcd_print("Mixpanel IoT Project");

		rpi.lcd_setCursor(0,1);
		rpi.lcd_print(moment().format("DD-MM-YYYY HH:mm"));

		rpi.lcd_setCursor(0,2);
		rpi.lcd_print(data1);

		rpi.lcd_setCursor(0,3);
		rpi.lcd_print(data2);

	}else {
		rpi.lcd_setCursor(0,0);
		rpi.lcd_print(data1);

		rpi.lcd_setCursor(0,1);
		rpi.lcd_print(data2);
	}
}

module.exports = rpi;
