# Home Assistant Water Pump
Within this Repo is code to allow a ESP8266 board to act as a IOT device to control a pump allowing for automated watering of a garden.


## Features


* water level sensor switch
* PWM controller for 12V pump


To connect to a Home Assistant server the following Arduino Library was used with it allowing for the creation of various devices through the Home Assistant MQTT integration. 


[Home Assistant Arudio Libiry](https://docs.arduino.cc/libraries/home-assistant-integration/)


Th schematic for the micro connections can be found in the supplied schematic in the repo.


For the other external components used a list can be seen below:


* [Ozito 36W 1700L/Hr Submersible Bilge Pump](https://www.bunnings.com.au/ozito-36w-1700l-hr-submersible-bilge-pump_p4819854)
* [12V 4.5Ah SLA Battery](https://www.jaycar.com.au/12v-4-5ah-sla-battery/p/SB2484?srsltid=AfmBOoqTauODHqt27cbil7ibFeK9lij9qtMcxgDwA6Qv8l0DqV2j-F-N)
* [3A PWM Solar Charge Controller 12V](https://www.jaycar.com.au/3a-pwm-solar-charge-controller-12v/p/MP3762)
* [KT Solar 10W 12V Solar Panel](https://www.bunnings.com.au/kt-solar-20-watt-12v-mono-crystalline-solar-panel-kt70716_p0510948)
