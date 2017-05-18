/**
      ntp4.ino example - This is the currently preferred starting point" 
      for a project requiring a RTC with ability to sync to an Internet
      based Time server. This is Ver 2.0, and works with an UNMODIFIED Sparkfun
      ESP8266WiFi .h and .cp files and a new derived class, ESP8266Class_a 
     that supplies missing UDP unctionality
      
      See 5/15/17 note in NTP3a.ino for historical perspective.
      (...libraries\ESP8266_KJF...)
 
      ToDo:
         05/18/17 - Gets timestamp ok from server.  Need to polish integraton
         with Time.h library.
      
*/

#include <Arduino.h> 
#include <SoftwareSerial.h>
#include <Time.h>
#include <SparkFunESP8266WiFi_a.h>  )

#define TIME_SERVER_IP         "216.229.0.179"   // ESP8266 doesn't support DNS lookup yet                                                                                        
#define TIME_SERVER_UDP_PORT   123               // Time server port 
#define CLIENT_UDP_PORT        13
#define TIMEOUT                10000             // RESET command, 10 sec
#define LOCAL_TIME_UTC_OFFSET  3UL               // Hours difference from UTC, as a positive value!
#define SYNC_TIME_INTERVAL     5UL               // Minutes (WAS 60UL)

ESP8266Class_a espa; // Derived from ESP8266Class (SparkFunESP8266WiFi.h

// Global variables
const int   NTP_PACKET_SIZE = 48; 
byte        packetBuffer[ NTP_PACKET_SIZE]; // Buffer for incoming and outgoing packets
time_t      serverLocalTimeNow;             // Unix time format
time_t      nexrClkCheckTime;               // Clock sync

 // send timestamp request datagram
 void sendTimeStampReq() {
   // set all bytes in the time request buffer to 0
   memset(packetBuffer, 0, NTP_PACKET_SIZE);
   // Initialize values needed to form NTP request
   // (see rfc 4330 for details on the packets)
   packetBuffer[0] = 0b11100011;   // LI, Version, Mode (OxE3)
   packetBuffer[1] = 0;     // Stratum, or type of clock
   packetBuffer[2] = 6;     // Polling Interval
   packetBuffer[3] = 0xEC;  // Peer Clock Precision
   // 8 bytes of zero for Root Delay & Root Dispersion
   packetBuffer[12]  = 49;
   packetBuffer[13]  = 0x4E;
   packetBuffer[14]  = 49;
   packetBuffer[15]  = 52;
   
  espa.sendBinPacket(packetBuffer, NTP_PACKET_SIZE);
 }       
         
 // Receive and process reaponse
 void recvResponse() {
   // The Server's 48-byte response packet follows this IPD string in 
   // the 8266 output stream
    espa.find("+IPD,48:");

   // Read the response from the server into the 48-byte buffer
   espa.readBytes(packetBuffer, 48);

   //The timestamp starts at byte 40 of the received packet and is four bytes,
   // or two words, long. First, extract the two words:
   unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
   unsigned long lowWord  = word(packetBuffer[42], packetBuffer[43]);

   // combine the four bytes (two words) into a long integer
   // this is NTP UTC time (seconds since Jan 1 1900):
   unsigned long secsSince1900 = highWord << 16 | lowWord;

   // now convert NTP time into Unix time:
   // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
   const unsigned long seventyYears = 2208988800UL;
   // subtract seventy years, convert to local time
   //unixTimeNow = secsSince1900 - seventyYears - (LOCAL_TIME_OFFSET * SECS_PER_HOUR); 
   serverLocalTimeNow = secsSince1900 - seventyYears - (LOCAL_TIME_UTC_OFFSET * SECS_PER_HOUR); 
   return;
 }     
         
// Sync s/w clock to time server
void syncClock() {  
      setTime(  hour(serverLocalTimeNow),
              minute(serverLocalTimeNow),
              second(serverLocalTimeNow),
                 day(serverLocalTimeNow),
               month(serverLocalTimeNow),
                year(serverLocalTimeNow));
} 

// Periodically check drift and sync the software clock.   
void isClockSyncDue() {     
   if(now() > nexrClkCheckTime){
      nexrClkCheckTime = (now() + (SYNC_TIME_INTERVAL * SECS_PER_MIN));
      sendTimeStampReq(); // Get fresh server time 
      recvResponse();     // This freshens serverLocalTimeNow 
      Serial.print(espa.getDateTimeStr(now()));
      time_t swClkTime = now();
      //time_t serverTime = second(serverLocalTimeNow);
      
      if(swClkTime == serverLocalTimeNow){
         Serial.println(F("S/W Clock and Standard Time Server AGREE within 1 sec. "));
      }
      else {
         if(swClkTime > serverLocalTimeNow) {
            Serial.print(F("Software clock LEADS Time Server by ")); 
            Serial.print(swClkTime - serverLocalTimeNow); Serial.println(F(" sec."));
         } 
         else {
            Serial.print(F("Software clock LAGS Time Server by ")); 
            Serial.print(serverLocalTimeNow - swClkTime); Serial.println(F(" sec."));   
         }

      }
       //syncClock(); // <------------------------ TEMPORARY DISABLE TO BETTER DETERMINE DRIFT RATE .
       //Serial.println("Software clock syncronized to Internet time server");   
   }
}

// Show compared timestamp
void printDebug() {
      Serial.print(F("    Time server now stamp: ")); Serial.println(espa.getDateTimeStr(now()));      
      Serial.print(F("   Software clk now stamp: ")); Serial.println(espa.getDateTimeStr(serverLocalTimeNow));         
}


void setup() {   
   Serial.begin(9600);
   espa.begin(); 

   // Asuume at this entry point the Sparkfun ESP8266 shield has already 
   // logged on to the LAN at power up. (AT_CWAUTOCONN=1) 
      
   if(espa.reset())
      Serial.println(F("Station RESET successful "));
  
   // Wait for IP to be restored (~5 sec)
   String buf2;
   uint16_t  timeup = millis() + TIMEOUT;
   bool targetFound = false;
   while(timeup > millis()) {
     buf2 = espa.captureResponse();
     if(buf2.indexOf(F("GOT IP")) > 0) {
        targetFound = true;
        break;
     }
   }   
   if(targetFound) {
     Serial.print(F("IP Restored: ")); 
     Serial.println(espa.localIP());
   }
   else {
     Serial.print(F("ESP8266 RESET TIMEOUT ERROR ")); 
     Serial.println();
   }  
   //delay(2000);
  
   // Start UDP session
   Serial.print(F("Start UDP session with Time server: "));
   if(espa.udpStart(TIME_SERVER_IP, TIME_SERVER_UDP_PORT, CLIENT_UDP_PORT)) 
      Serial.println(F("SUCCESS!")); 
   else 
      Serial.println(F("UDP startup FAILED!"));
   
   // Initial s/w clock syncronization
   sendTimeStampReq();  
   recvResponse();
   syncClock();
   Serial.print(F("Initial sync result: ")); Serial.println();   
   printDebug();
   nexrClkCheckTime = (now() + (SYNC_TIME_INTERVAL * SECS_PER_MIN));
   
}


void loop() {
   
   // Check need for periodic clock syncronization
   isClockSyncDue();   
   
   
  // Pass through
  while (Serial.available())
    espa.write(Serial.read());
  while (espa.available())
    Serial.write(espa.read());
 
}
  