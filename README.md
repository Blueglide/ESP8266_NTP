# Sparkfun ESP8266 Shield
Add UDP support to Sparkfun library plus NTP example

One of the first projects completed after purchasing a <a href="https://www.sparkfun.com/products/13287">Sparkfun ESP8266 Arduino shield</a> was developing an example sketch that gave the Arduino UNO Real Time Clock capability, and maintained time sync with one of the timestamp servers available on the Internet. 

Although the published Sparkfun library ESP8266WiFi has good TCP support, a small number of additions are required for projects requiring simpler UDP functionality. 

A new ESP8266Class_a class was derived using the Sparkfun ESP8266Class class as the base class and provides additional UDP and other useful time-related methods. 

In addition an example NTP sketch is provided to illustrate how it all fits together. It periodically syncs the software clock in Michael Margolis' Arduino Time library with an Internet based Time Server and keeps a record of any observed drift over time. (See the User Interface UserScreenCapture.jpg file.)

All required files will be posted here shortly.











