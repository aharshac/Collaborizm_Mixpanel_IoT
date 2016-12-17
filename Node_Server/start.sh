#!/bin/sh

cd /home/pi/mixpanel_iot_server/

# Using nodemon
sudo npm run dev

# Using forever
#sudo forever start -l forever.log -e mixpanel_err.log index.js

# To run, Use
# sudo /home/pi/shared/mixpanel/./start.sh

# @reboot sleep 30; lxterminal -e sudo /home/pi/shared/mixpanel/./start.sh

#$SHELL
