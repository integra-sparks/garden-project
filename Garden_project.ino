#include <Arduino.h>
//#include <Wire.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
//#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>
//#include <SPI.h>               // include Arduino SPI library 
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

const char* ssid     = "BT-FRA6JT"; // Your ssid
const char* password = "VbyURH9ACuXvL6"; // Your Password

WiFiUDP Udp;
unsigned int localPort = 2900;  // local port
//const char* ssid     = "TALKTALK-02798A";
//const char* password = "8X8GV8PH";
//const char* ssid     = "dlink-E004";
//const char* password = "iyegg96982";
// NTP Servers:
//static const char ntpServerName[] = "us.pool.ntp.org";
static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";
//0.uk.pool.not.org
IPAddress NTP1(143,210,16,201);
IPAddress NTP2(178,79,160,57);
IPAddress NTP3(217,114,59,3);
IPAddress NTP4(87,117,251,3);

IPAddress ip(192,168,1,100);
WiFiServer server(80);
IPAddress gateway(192, 168, 1, 254); // set gateway
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
 
const int timeZone = 1;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

#define valve_1 16
#define valve_2 14
#define valve_3 12
int      HrsCpy ;//
int      MinCpy ;//
int      SecCpy ;//
int BAY=1;
int      Water=LOW;
int      Amountbay1init=1200;
int      Amountbay2init=1200;
int      Amountbay3init=0;
int      Amountbay1=60;
int      Amountbay2=60;
int      Amountbay3=60;
String  Amountbaychar;
int oldDay=1000;
int oldHour=1000;
int oldSecond=1000;
int bay_1_On=LOW;
int bay_2_On=LOW;
int bay_3_On=LOW;
long elapsed;
String SS1;
String SM1;
String SE;
String Resultbay1;
String SS2;
String SM2;
String Resultbay2;
String SS3;
String SM3;
String Resultbay3;
const int packetSize2 = 6;
byte packetBuffer2[packetSize2];
String msgCpy="X";
char CpyCpy [8];
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
int NTPavail=HIGH;

#define RelayPIN 0           // Relay  is connected to ESP8266 pin GPIO1 (TX)
String readString;
char linebuf[80];
int charcount=0;

//////////////////////////////////////////////
//void digitalClockDisplay();
//void printDigits(int digits);
//void sendNTPpacket(IPAddress &address);
/*-------- NTP code ----------*/
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
//////////////UDP r

time_t getNtpTime()
{
 // IPAddress ntpServerIP; // NTP server's ip address
 IPAddress NTP2; // NTP server's ip address
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
 // WiFi.hostByName(ntpServerName, ntpServerIP);
 // Serial.print(ntpServerName);
  Serial.print(": ");
//  Serial.println(ntpServerIP);
 // sendNTPpacket(ntpServerIP);
 sendNTPpacket(NTP3);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      NTPavail=LOW;
    }
  }
  Serial.println("No NTP Response :-(");
  NTPavail=0;
  return 0; // return 0 if unable to get the time
}
void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}
void updateTempTime()
{
   HrsCpy=hour();;//
   MinCpy=minute();//
   SecCpy=second();//
}

void setup() {
    pinMode(valve_1,OUTPUT);
  pinMode(valve_2,OUTPUT);    
  pinMode(valve_3,OUTPUT); 
Serial.begin(115200);
Serial.flush();
Serial.println();
delay(100);

WiFi.config(ip, gateway, subnet); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
      {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
      }

  

Serial.println();
Serial.println("Starting UDP");
Udp.begin(localPort);
Serial.print("Local port: ");
Serial.println(Udp.localPort());

  // initialize the display

Serial.println("");
Serial.println("WiFi connected");
Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
// Start the server
server.begin();
Serial.println("Server started");
// Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("GardenWater");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");
  ArduinoOTA.setPassword((const char *)"8266");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  }); 
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
 Serial.println("OTA Ready");
 
Serial.println("waiting for sync");
setSyncProvider(getNtpTime);
setSyncInterval(500000);
delay(200);
 time_t getNtpTime();
SE=" Mins</a>";
SS1="<a href=\"/? \"\"><button8 style='font-size:200%; background-color:d#DDDDFF; color:green;border-radius:35px; position:absolute; top:250px; left:1100px;'>";
SS2="<a href=\"/? \"\"><button9 style='font-size:200%; background-color:d#DDDDFF; color:green;border-radius:35px; position:absolute; top:300px; left:1100px;'>";
SS3="<a href=\"/?\"\"><button10 style='font-size:200%; background-color:d#DDDDFF; color:green;border-radius:35px; position:absolute; top:350px; left:1100px;'>";

} 
void loop() {
 
///////////////
WiFiClient client = server.available();
 if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            ///////////////////////

 Serial.println("new client");
    charcount=0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
            if (client.available()) {
        char c = client.read();
     
        //read char by char HTTP request
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
          //Serial.print(c);
         }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) 
        {
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta/n charset='UTF-8',name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.print("<meta http-equiv=\"refresh\" content=\"100000\">");;

          break;
        }

        client.println("");
        if (c == '\n') {
      
          // you're starting a new line
          currentLineIsBlank = true;
          charcount=0;    
    
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(2);
            //////////////////////
           
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta/n charset='UTF-8',name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
   //         client.println("<link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>"); ;
            client.println("<body>");;
          client.println("<h1><font size='8' color='red'>Garden Watering System</font></h1>");  
///////////////////////////////////////////////////////////////////

////////////////////////////////////////////////
// Generates buttons to control the relay


            client.println("<pre>");
            client.println("<a href=\"/?button5\"\"><button11 style='font-size:250%; background-color:black; color:white;border-radius:35px; position:absolute; top:150px; left:750px;'>STOP WATERING</a>");
           client.println("<a href=\"/?button1on\"\"><button1 style='font-size:250%; background-color:yellow; color:green;border-radius:35px; position:absolute; top:250px; left:500px;'>Bay 1 +</a>");
           client.println("<a href=\"/?button1off\"\"><button2 style='font-size:250%; background-color:yellow; color:green;border-radius:35px; position:absolute; top:250px; left:760px;'>Bay 1 -</a>");

           client.println("<a href=\"/?button2on\"\"><button3 style='font-size:250%; background-color:yellow; color:green;border-radius:35px; position:absolute; top:300px; left:500px;'>Bay 2 +</a>");
           client.println("<a href=\"/?button2off\"\"><button4 style='font-size:250%; background-color:yellow; color:green;border-radius:35px; position:absolute; top:300px; left:760px;'>Bay 2 -</a>");     
   
           client.println("<a href=\"/?button3on\"\"><button5 style='font-size:250%; background-color:yellow; color:green;border-radius:35px; position:absolute; top:350px; left:500px;'>Bay 3 +</a>");
           client.println("<a href=\"/?button3off\"\"><button6 style='font-size:250%; background-color:yellow; color:green;border-radius:35px; position:absolute; top:350px; left:760px;'>Bay 3 -</a>");

           client.println("<a href=\"/?button4\"\"><button7 style='font-size:250%; background-color:black; color:yellow;border-radius:35px; position:absolute; top:450px; left:750px;'>REAL TIME UPDATE </a>");
           client.println("</pre>");

         client.print("</body>\n</html>");
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } 
        
              client.println("");    
            if (readString.indexOf("?button5") >0){
            Amountbay1=0;
            Amountbay2=0;
            Amountbay3=0;
     
               
           }
          else   if (readString.indexOf("?button1on") >0){
                Amountbay1=Amountbay1+60;
         //  client.println(Amountbay1);
           Serial.println(Amountbay1);   
                       
           }
         else  if (readString.indexOf("?button1off") >0){
                 if(Amountbay1>=60){ Amountbay1=Amountbay1-60;}
                 else{Amountbay1=0;}
            //      client.println(Amountbay1);
                  Serial.println(Amountbay1);
                  
           }
       else    if (readString.indexOf("?button2on") >0){
                Amountbay2=Amountbay2+60;
        //   client.println(Amountbay2);
           Serial.println(Amountbay2);
              
           }
       else    if (readString.indexOf("?button2off") >0){
               if(Amountbay2>=60){ Amountbay2=Amountbay2-60;}
               else{Amountbay2=0;}
           //       client.println(Amountbay2);
                  Serial.println(Amountbay2);
                   
           }
        else    if (readString.indexOf("?button3on") >0){
                 Amountbay3=Amountbay3+60;
         //  client.println(Amountbay3);
           Serial.println(Amountbay3);
              
           }
        else   if (readString.indexOf("?button3off") >0){
                 if(Amountbay3>=60){ Amountbay3=Amountbay3-60;}
                 else{Amountbay3=0;}
             //     client.println(Amountbay3);
                  Serial.println(Amountbay3);
                     
           }
        else   if (readString.indexOf("?button4") >0){
             client.println("<!DOCTYPE html><html>");
            client.println("<head><meta/n charset='UTF-8',name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
             client.print("<meta http-equiv=\"refresh\" content=\"2\">");
    
           }

SM1=String(Amountbay1/60);
Resultbay1=SS1+SM1+SE;
SM2=String(Amountbay2/60);
Resultbay2=SS2+SM2+SE;
SM3=String(Amountbay3/60);
Resultbay3=SS3+SM3+SE;
    client.println(Resultbay1);
    client.println(Resultbay2);
    client.println(Resultbay3);
    // Clear the header variable
   
    // Close the connection
    client.stop();
    readString="";
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  if (SecCpy!=second()){updateTempTime();SecCpy=second();}
if(oldDay!=day()){oldDay=day();Amountbay1=Amountbay1init;Amountbay2=Amountbay2init;Amountbay3=Amountbay3init;}//Amount is in seconds
if(hour()>=8&&hour()<=22)
  {
  
  Water=HIGH;
if (Water&&oldSecond!=second())
{
switch(BAY)
    {
    case 1:
    if(Amountbay1>=1){Amountbay1--;oldSecond=second(); digitalWrite(valve_1,HIGH);Serial.print("Bay 1 Watering ");digitalClockDisplay();}
    else{BAY=2;digitalWrite(valve_1,LOW);}
    break;
    case 2:
    if(Amountbay2>=1){Amountbay2--;oldSecond=second(); digitalWrite(valve_2,HIGH);Serial.print("Bay 2 Watering ");digitalClockDisplay();}
    else{BAY=3;digitalWrite(valve_2,LOW);}
    break;
    case 3:
    if(Amountbay3>=1){Amountbay3--;oldSecond=second(); digitalWrite(valve_3,HIGH);Serial.print("Bay 3 Watering ");digitalClockDisplay();}
    else{BAY=1;digitalWrite(valve_3,LOW);}
    break;
    default:
    Serial.println("should not be here");
    BAY=1;
    break;
    }
}//of watering section
else{Water=LOW;}
  }//of hour
else{digitalWrite(valve_1,LOW);digitalWrite(valve_2,LOW);digitalWrite(valve_3,LOW);}
//////////////////////////////////////
  ArduinoOTA.handle();
/////////////////////////////////////
}
