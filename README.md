# A Smart Home system facilitating the operation of a heating furnace

## What is this?
This repository includes code used for making a weather station and dashboard for monitoring my central heating furnace. No setup instructions are provided, as this is very much a one-off project, tailor-made for my needs. It's here as a demonstration only, though I will provide assistance if you want to do something similar.

### Weather station features
- powered by an ESP8266
- gets data through MQTT from a central processing node
- RGB diode changes colour depending on the furnace's temperature
- LCD screen presenting data in several ways - in a tabular, key-value format, and with colored squares/a warning sign depending on the value of various measurements, like inside temperature and humidity, furnace and boiler temperature, and outside air quality
- 3d-printed case

![PIC1](https://github.com/Syverynx/Smart-Home-System/blob/master/Images/Station.png)

![PIC2](https://github.com/Syverynx/Smart-Home-System/blob/master/Images/Station_screen.jpg)
### Temperature measurement 
- for cost and challenge reasons, powered by a ESP01
- measures temperature and sends it to a central node
- like the weather station, shows the current temperature of the measured system with a RGB diode

![PIC3](https://github.com/Syverynx/Smart-Home-System/blob/master/Images/Temperature_node.png)
### Central node
- it's a Raspberry Pi, with Node-RED and Mosquitto installed
- gathers data from measurement nodes, weather data from Airly through their API, processes the data and sends it to the weather station
- hosts a dashboard [(with node-red-dashboard)](https://flows.nodered.org/node/node-red-dashboard), which includes time-series graphs of abovementioned measurements
- was supposed to persist the data to InfluxDB, but I had gas heating installed before I could get to it

![PIC4](https://github.com/Syverynx/Smart-Home-System/blob/master/Images/Dashboard.png)