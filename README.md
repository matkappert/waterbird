# Waterbird 🦩

Water irrigation controller using weather data from Node-Red to Arduino based microcontrollers with MQTT



## Setup


### Step 1: Install Node-Red

[Node-Red Setup](https://nodered.org/docs/getting-started/raspberrypi)

Running the following command will download and run the script
~~~bash
bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/update-nodejs-and-nodered)
~~~

Running as a service
~~~bash
node-red-start
~~~
this starts the Node-RED service and displays its log output. Pressing Ctrl-C or closing the window does not stop the service; it keeps running in the background


### Step 2: Install Mosquitto (MQTT client)

[Mosquitto Setup](https://learn.adafruit.com/diy-esp8266-home-security-with-lua-and-mqtt/configuring-mqtt-on-the-raspberry-pi)

The first step I would recommend is updating the software on your Raspberry Pi. Open up a terminal and enter the following commands:
~~~bash
sudo apt-get update
sudo apt-get upgrade
sudo reboot
~~~

Once your Pi has rebooted, you can install our MQTT broker Mosquitto, again in the terminal type:
~~~bash
sudo apt install -y mosquitto mosquitto-clients
~~~


find you Pi IP Address
~~~bash
hostname -I
~~~



