# node.js ESP8266 emulator

This subproject is an emulator for a ESP8266 device written in JavaScript. It only supports a subset of the commands of the real ESP8266, because the WIFI commands dont make much sense here. 


## How to run

Type `node app.js` to run the server. To use with dosbox (which is the intended use of the emulator), configure the Serial port:
`serial1=nullmodem server:127.0.0.1 port:7000 transparent:1 rxdelay:0`