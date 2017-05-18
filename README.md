# Sparkfun ESP8266 Shield
Add UDP support to Sparkfun library plus NTP example

One of the first projects I wanted to complete after purchasing a <a href="https://www.sparkfun.com/products/13287">Sparkfun ESP8266 Arduino shield</a> was a illustrative example sketch that gave the Arduino UNO Real Time Clock capability and maintained time sync within +/- one sec to one the of timestamp servers available on the Internet. 

Although the published Sparkfun library ESP8266WiFi has good TCP support, a few additions are required for projects using simpler UDP functionality. 

A new ESP8266Class_a class was derived using the Sparkfun ESP8266 as the base class to provide a few additional UDP-specific and other useful methods. 

In addition an example NTP example sketch is provided to illustrate how it all fits together. It periodically syncs the software clock in Michael Margolis' Arduino Time                        library with an Internet based Time Server and keeps a record of any observed drift over time. 












