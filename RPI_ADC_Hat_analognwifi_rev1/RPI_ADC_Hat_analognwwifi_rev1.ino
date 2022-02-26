/*
 * MIT License

Copyright (c) [2022] [Seenov inc.]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 * */

/*Program & Source Code: Raspberry Pi ADC with ESP32C3
  The Program is designed to follow the below steps.

  On First boot, ESP32 will be in set up Station mode, and Read the pre-registered SSID and password combination and tries to connect to the same.

  If this process fails, it sets the ESP into Access Point mode and creates Open Wifi(Not protected with Password);

  This open wifi will allow the user to connect Using any Wi-Fi enabled device with a browser. the SSID is ADC_xxyyzz xx-zz last 3 mac adress digits

  After establishing a connection with the Access point, you can go to the default IP address 192.168.4.1 to open a web page that allows user to configure your SSID and password;

  Once a new SSID and password is set, the ESP reboots and tries to connect;

  If it establishes a connection, the process is completed successfully. 

  If the connection fails again, it will be set up as an Access Point to configure the new SSID and Password.

 Once connected it waits for UDP messages from Raspberry Pi running node-red  UPD send port 5045   receive on port 5044 
  
  In node-red    a broadcast UDP message is sent. The ADC responds with its IP address so Node-Red can now talk directly to the ADC.
***********************
********* use json for config and data retrieval ******************
* ******   ADC json config   object
    ref how the ESP32 adc works https://www.youtube.com/watch?v=RlKMJknsNpo&t=145s  very similar to the ESP32C3 ADC
    
  type :   conf  or query         type of message received  conf is used to setup ADC   query to request data  the host must request data
  ch:      channel number 0 to 5  total 6 channels config each seperatly ch 6 is adc2 ch0 = battery
  enable:  1 = enabled, 0 disable
  bits:     always 12  for now 0 used or not present = use default, can be 8-12
  samples: 1 no average, can be 4, 8 or 16 up to 255  make average # readings and calculate average
  att:     attenuation   Sets the input attenuation, default is ADC_11db, range is ADC_0db, ADC_2_5db, ADC_6db, ADC_11db
  msec:    millisec between reads,
   ///  not used, set up by default  batt:    report battery voltage  if=1 means battery present batt is manually connected to 1 adc channel, user defined must do conv on that channel to get
           batt voltage
  data:    16 bit results
   

  Query from RPI will look like this: char json[] = "{\"type\":\"conf\",\"ch\":0,\"enable\":1,\"bits\":12,\"samples\":1,\"att\":\"ADC_11db\",\"msec\":0,}";
  response will look like this


  type = conf config, query or LP low power mode
  response from ADC to aconfig
  char json2[] = "{\"type\":\"conf\",\"msg\":\"OK\"}";
   response to a conf
   type:  conf
    msg:    "OK", or "error msg"

   responce to query
   char json2[] = "{\"type\":\"query\",\"data\":\"ADC_x data\",\"msg\":\"chan number\" }";
   type: query
   data: conversion results 16 bits
   msg: error message  or channel number
   
   revision history
   14 Jan 2022  fixed getting credeentials form web page aswitching to station mode once finished
   need to add possibility of reset credentials with GPIO or message rev 0.1 to 0.5

  31 Jan adc works but wifi does not wifi works on esp32 only  asyncwifi does not work
  page style reboots all the time
  found reboot error was pin 16 as io out but asyncwifi needs to be changed rev 0.6,0.7

  1 Feb 2022   integrated new wifi web server to collect wifi ssid and psw to be tested rev 0.8
  2 Feb tested web server credentials, works well  next step communications with Raspberry Pi
    also more comments to be added  rev 0.9

   3 Feb to be added  The logo would be nice also <style>body{ width: 340px; margin:10; background-color:#81ACC5;}</style> 
   Added UDP link manageement link up  down tbd. Resolved reboot on too short msg from RPI
   Problem with json, cannot convert to json on RPI and deserialization error on ESP32
   4 feb added json error message now messages are deserialized correctly
   this is rev 0.10
   Made some progress json decoding verry tricky, if the proprety does not exist, you get unpredictable results
   including reboot
   6 feb works, release 1.0
   
   
*/
#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "AsyncUDP.h"
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include "ArduinoJson.h"
//#include <NoDelay.h>
#include <Update.h>
//#include "EEPROM.h"

//Variables
uint8_t i = 0;
uint8_t GPIO_LED = 8;
uint8_t LED_state = 1;
uint8_t AccesPointMode = 0;
uint8_t testBufffer[8] = {1, 1, 1, 1, 1, 1, 1, 1};
char channel_number;
int chan_enable[6]= {1,1,1,1,1,1};
char NumOfSamples_per_Avrg[6]={16,16,16,16,16,16};
int  msBetweenSamples[6]={10000,0,0,0,0,0};
int  chan_att[6];
int  chan_coversionResults[6];
int  chan_avrgResults[6]={0,0,0,0,0,0};
char counting[6] = {0, 0, 0, 0, 0, 0,};
double  chan_summ[6];
char channelToRead = 0;
int statusCode;
const char* ssid = "Seenov24_2_4";
const char* password = "bandittaichi";
String localwifissid= "..................................."; // reserve 35 characters
String localwifipsw= "...................................";
String st;
String content;
String esid;
String hostname = "SEENOV";
const char* host = "seenov";
String epass = "";
char rebootNow = 0;
char softReboot = 0;
byte mac1[6];
char newssid[] = "ADC_123456";
char UDP_ch_active =0;
char ascii_mac[19] = {};
char temp_string[255] = {};
String testmac = "3c:61:05:0c:85:a0";
IPAddress ReceivedFromIP;
IPAddress ip;

#define INTERVAL_1 5000
#define INTERVAL_2 86400000  // adc2_0   set to 24hrs
#define INTERVAL_3 11000
#define INTERVAL_4 9000

StaticJsonDocument<1000> doc_r;
StaticJsonDocument<1000> doc_s;
char  fromString[210];   // for udp send
uint8_t udpString[210];  // for udp send
int SendLen;
unsigned long time_1 = 0;
unsigned long time_2 = 0;
unsigned long time_3 = 0;
unsigned long time_4 = 0;
 
 

//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);
void blinkLED();

WebServer server(80);   // used to input network credentials 
AsyncUDP udp;



void setup()
{

  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  delay(1000);
  Serial.println();
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect(); // ensure a clean start
  EEPROM.begin(512); //Initialasing EEPROM
  delay(1000);
  
  pinMode(GPIO_LED, OUTPUT);
  digitalWrite(GPIO_LED, HIGH);   // off
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  /******************************************** 
   *  Read eeprom to retreive saved ssid and pass
   */

  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  //esid="test";  // Used for testing to force bad ssid and psw
  Serial.println();
  Serial.print("SSID:");
  Serial.println(esid);


  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  //epass="test";// used for testing
  Serial.print("PASSWORD:");
  Serial.println(epass);

  /******************************************************
   * 
   * Try to connect to local wiFi with saved ssid psw
   * 
   ***************************************************/
  WiFi.begin(esid.c_str(), epass.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac1); // returns mac address
  
  char temp = 30;    // seconds tying to connect

  //  wait up to 30 sec  for connection
  while ((WiFi.status() != WL_CONNECTED)  && (temp > 0) )
  {
    Serial.print("nc");  // no connection
    delay(1000);
    temp--;
  }

  // after wait if still not connected launch web for new credentials
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connection Status Negative");
    Serial.println("Turning On the HotSpot");
    setupAP();// Setup HotSpot to get new SSID and PSW
    AccesPointMode = 1;
    digitalWrite(GPIO_LED, LOW);
  }
  else      // here wifi connect successful 
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.print(esid);
    Serial.println(" Successfully");
    AccesPointMode = 0;
    digitalWrite(GPIO_LED, HIGH);
  }
/*******************************************
 * 
 *   Successful connexion, check UDP then
 *   fall through to main Loop
 ******************************************/

    if((udp.listen(5044)) && ( AccesPointMode==0)) {
    Serial.print("UDP Listening  ");
    udp.onPacket([](AsyncUDPPacket packet) {
        //Serial.print("UDP Packet Type: ");
        //Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
        //Serial.print(", From: ");
        ReceivedFromIP = packet.remoteIP();  // needed for reply
       // Serial.print(packet.remoteIP());
        //Serial.print(":");
        //Serial.print(packet.remotePort());
        //Serial.print(", To: ");
        //Serial.print(packet.localIP());
        //Serial.print(":");
        //Serial.print(packet.localPort());
        Serial.print(", Length: ");
        Serial.print(packet.length());
        Serial.println(", Data: ");
        Serial.write(packet.data(), packet.length());
        Serial.println();
        // read packet and take action
        //ReceivedFromIP = packet.remoteIP();

        packet.printf("Got %u bytes of data", packet.length());
        //parse json msg  after copy to buff
       
        char tempBuff[packet.length()+10];
        memcpy(tempBuff, packet.data(), packet.length());
       
        DeserializationError error = deserializeJson(doc_r, tempBuff);
        
         
        if (error) {
          Serial.print(F("deserializeJson() failed with code "));
          Serial.println(error.f_str());
          
        }else {               // no error use the values received
    /****************************************************
     *         Decode received message             
     *         tbd add  ADC identifyer so multiple 
     *         can communicate with Raspberry Pi
    ****************************************************/
         const char* type1 = doc_r["type"];
         const char* ADC = "ADC1";
         Serial.println(type1);
         if (strcmp(type1,  ADC)==0 )
        {
            Serial.println("received request for connection");
             UDP_ch_active=1;
        }
            
         const char* category = doc_r["category"];
         const char* settingstype = "settings";
         Serial.println(category);
         if (strcmp(category,  settingstype)==0 )
        {
            Serial.println("received settings");
           // delay between sample bursts
           msBetweenSamples[0]=(int)doc_r["delay"];  //only one delay for all channels
            // Enable / disable channels 
         
            chan_enable[0] = (int)doc_r["ench0"]-99;
            chan_enable[1] = (int)doc_r["ench1"]-99;
            chan_enable[2] = (int)doc_r["ench2"]-99;
            chan_enable[3] = (int)doc_r["ench3"]-99;
            chan_enable[4] = (int)doc_r["ench4"]-99;
            chan_enable[5] = (int)doc_r["ench5"]-99;
           

       /****************************************************
        *  tbd 
        *     Set attenuation TBD
        *     
         chan_att[0] = (int) doc_r[" xxx0"];    xx0 needs to be defined in Node-Red
         chan_att[1] = (int) doc_r[" xxx1"];
           etc for remaining channels

        *******************************************************/    

        }
        }

      });
    }
  

  // Print usefull information for debugging
  String mac2 = (String)WiFi.macAddress() ;
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(mac2);
  if(AccesPointMode ==0)
  {
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
  }
  
   
  
 /**********************************************************************
  *   tbd   configured from Node-red messages see earlier lines 332 and later lines 798
  *    Init all adc channels adc1 and adc 2
  *    ADC2 tbd
  *    
  ************************************************************************/
  adcAttachPin(0);
  adcAttachPin(1);
  adcAttachPin(2);
  adcAttachPin(3);
  adcAttachPin(4);
 // adcAttachPin(5);              //confirm how to do this for ADC2 ch0
  analogSetPinAttenuation(0, ADC_11db);
  analogSetPinAttenuation(1, ADC_11db);
  analogSetPinAttenuation(2, ADC_11db);
  analogSetPinAttenuation(3, ADC_11db);
  analogSetPinAttenuation(4, ADC_11db);
  //analogSetPinAttenuation(5, ADC_11db);  // confirm how to do this for ADC2 ch0

}


void loop() {


  if (AccesPointMode == 1)
  {
   // blinking.update();
    server.handleClient();
   
  }

 
  /****************************************
   * tbd
   * Function to force get new chredentials by software
   * Add Node-red object  that will be decoded 
   * 
   ***************************************/
  if (softReboot == 1)
  {
    softReboot = 0;
    Serial.println("Turning the HotSpot On");
    setupAP();// Setup HotSpot to get new SSID and PSW
    AccesPointMode = 1;
    digitalWrite(GPIO_LED, LOW);
  }
  
     /****************************************
   * tbd
   * Function for over the air programming
   *  
   * 
   ***************************************/
     /*
      * ???
      */



  
 

    /*****************************************************************************
    * 
    *    Create and Send udp json packet with ADC values  in int32 mV format
    *    samples are average of 16 samples  
    *    important no output untill receive from udp to find ip and port to send to
    *    2 json doc  doc_r receive  and doc_s send
    *  
    ************************************************************************************/
    if(millis() >= time_4 + INTERVAL_4)
    {
        time_4 += INTERVAL_4;
        // always send all 6 channels, only 5 for this example, later add status etc
        doc_s["type"] = "ADC1";  // tbd  better define packet content
        doc_s["ch0"] = chan_avrgResults[0];
        doc_s["ch1"] = chan_avrgResults[1];
        doc_s["ch2"] = chan_avrgResults[2];
        doc_s["ch3"] = chan_avrgResults[3];
        doc_s["ch4"] = chan_avrgResults[4];
        // Send the data over UDP if  active
        if(UDP_ch_active==1)
        {
            serializeJson(doc_s, fromString);
            SendLen = strlen( fromString );
            memcpy( udpString, fromString, SendLen );
            Serial.print("sending message to: ");
            Serial.println(ReceivedFromIP);
            Serial.print("ch 0 mV: ");
        Serial.print(chan_avrgResults[0]);
        Serial.print("    ch 1 mV: ");
        Serial.print(chan_avrgResults[1]);
        Serial.print("    ch 2 mV: ");
        Serial.print(chan_avrgResults[2]);
        Serial.print("    ch 3 mV: ");
        Serial.print(chan_avrgResults[3]);
        Serial.print("    ch 4 mV: ");
        Serial.println(chan_avrgResults[4]); 
            AsyncUDPMessage msg(strlen(fromString));
            msg.write( udpString, SendLen );
            udp.sendTo( msg, ReceivedFromIP, 5045 );
        }
   }

  

  /***************************************************************
   * tbd fix bug  aanalogReadMillivolts  is not using calibration settings
   * 
   * Do conversions for active channels 0 to 4 (ADC 1)
   * Sum 16 conversions per enabled channel 
   * NumOfSamples_per_Avrg[0] sets average for all channels
   * Conversions will faster if less channels enabled
   * Calculate average after all samples are done
   * 
   * 
   ***************************************************************/
   // msBetweenSamples[0]= 10000;   // for test comment out
    if(millis() >= time_1 + msBetweenSamples[0])
    {
      time_1 +=msBetweenSamples[0];
         
        
      for (char samples1=0; samples1<NumOfSamples_per_Avrg[0]; samples1++)
      {
          for(char ii=0; ii<5;ii++)
          {
            if (chan_enable[ii]==1)  // if active convert
            {
                chan_summ[ii]+=analogReadMilliVolts(ii);
            }
          }
      }
          for(char ii=0; ii<5;ii++)
          {
            if (chan_enable[ii]==1)  // if active convert
            {
               chan_avrgResults[ii]= chan_summ[ii]/NumOfSamples_per_Avrg[0];
               chan_summ[ii]=0;
            }
            else 
            {
              chan_avrgResults[ii]= 0;
              chan_summ[ii]=0;
            }
          }     
                 
        
    }
     //   Other low priority tasks
      if(millis() >= time_2 + INTERVAL_2){
        time_2 +=INTERVAL_2;
        /***********************
         *  TBD   fix bug   ADc2_ch0  does not work creates reboot
         *   note chan enable [5], chan sum[5]  etc are adc2_0
         */
        /* 
         *  
         if (chan_enable[5]==1)  // if active convert
        {
        WiFi.disconnect(); // ensure a clean start
        adcAttachPin(5);              //confirm how to do this for ADC2 ch0
        analogSetPinAttenuation(5, ADC_11db);  // confirm how to do this for ADC2 ch0
        for (char samples1=0; samples1<NumOfSamples_per_Avrg[0]; samples1++)
        {
            chan_summ[5]+=analogReadMilliVolts(5);
        }
        chan_avrgResults[5]= chan_summ[5]/NumOfSamples_per_Avrg[0];
        chan_summ[5]=0;
      
        //Serial.println(analogReadMilliVolts(5));
        for (int i = 0; i < 32; ++i)
        {
        esid += char(EEPROM.read(i));
        }
        for (int i = 32; i < 96; ++i)
        {
        epass += char(EEPROM.read(i));
        }
        WiFi.begin(esid.c_str(), epass.c_str());
        WiFi.mode(WIFI_STA);
        }
        
        */
     }
   
    if(millis() >= time_3 + INTERVAL_3){
        time_3 +=INTERVAL_3;
       

    }
   
    
}

/*******************************************************************
* 
*     Fuctions used for WiFi credentials saving and connecting
*   to it which you do not need to change
*   
*******************************************************************/


bool testWifi(void)
{
  int c = 0;
  //Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

/*******************************************************************
*   
*   
*   Start the web server
*   
*   
*   
*******************************************************************/
  void launchWeb()
  {
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi connected");
  }
   createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
  }

/*******************************************************************
*    
*    
*    Set up Access Point so user can connect as a client and access web server
*    
*    
*******************************************************************/
void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  /*    use to display scan on credential update page
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  */
  delay(100);
  WiFi.macAddress(mac1);
  //   ****************  we set a new ssid ADC_ +  last 3 parts of mac **********
  //   
    myitoa();
   Serial.println(newssid);
   
  WiFi.softAP(newssid,"");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Connect to SSID ADC_xxxxx");
  Serial.println("Go to web page and update SSID and PASW for your network");

}

/*******************************************************************
*     
*     Create web server to input Local WiFi credentials
*    
*     
*     
*      
*  
*******************************************************************/
  void createWebServer()
  {
  {
    server.on("/", []() {
      
     // IPAddress ip = WiFi.softAPIP();
     // String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE html> <html>  <head><meta ame=  'viewport  ' content=  'width=device-width, initial-scale=1.0, user-scalable=no  '>  ";
 content+= " <title>ESP WiFi Manager</title>   <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
 content+= " body{margin-top: 50px;} h1 {color: #444444;margin: 30px auto 30px;}   h3 {color: #ffffff;margin-bottom: 50px;}   label{display:inline-block;width: 160px;text-align: center;font-size: 1.5rem; font-weight: bold;color: #ffffff;}";
 content+= " form{margin: 0 auto;width: 360px;padding: 1em;border: 1px solid #CCC;border-radius: 1em; background-color: #077c9f;}    input {margin: 0.5em;font-size: 1.5rem; }  "; 
   content+= "   .styled {    border: 0;    line-height: 2.5;    padding: 0 20px;    font-size: 1.5rem;    text-align: center;    color: #fff;    text-shadow: 1px 1px 1px #000; ";
  content+= "  border-radius: 10px;    background-color: rgba(220, 0, 0, 1);    background-image: linear-gradient(to top left,  rgba(0, 0, 0, .2), rgba(0, 0, 0, .2) 30%, ";
  content+= "   rgba(0, 0, 0, 0));    box-shadow: inset 2px 2px 3px rgba(255, 255, 255, .6),   inset -2px -2px 3px rgba(0, 0, 0, .6);.styled:hover {   background-color: rgba(255, 0, 0, 1);} ";
content+= ".styled:active {    box-shadow: inset -2px -2px 3px rgba(255, 255, 255, .6), inset 2px 2px 3px rgba(0, 0, 0, .6);}  </style>  "; 
content+= "  <meta charset=  'UTF-8  '>    </head>   <body>   <h1>Welcome to WiFi Update</h1>    <h1>ESP32C3 6 Channel ADC</h1> ";  
content+= "       <h3>Enter local WiFi SSID&PSW</h3>    <form method='get' action='setting'> <div><label>WiFi SSID</label> <input name= 'ssid' length= 32></div> ";  
 content+= "   <div><label>WiFi Password </label> <input name=  'pass'  length= 32></div>  "; 
 content+= " <h3>Please Double Check Before Saving</h3>   <button class= 'favorite styled ' type= 'submit'>    SAVE</button> </form>    </body>  </html>  ";

      server.send(200, "text/html", content);
    });
   
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("new values submitted");
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();
         // check  this part
         
        content = "<!DOCTYPE HTML><head><meta name='viewport' content='width=device-width,initial-scale=1'/>";
        content += "<style>body{ width: 340px; margin:10; background-color:#81ACC5;}</style></head><html>";
        content += "<h2>Success<br>SSID & PSW saved to eeprom<br> Rebooting and connecting to new wifi<br></h2></html>";
        statusCode = 200;
        server.send(statusCode, "text/html", content);
        delay(1000);
        ESP.restart();
      } else {
        // to be checked
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(statusCode, "application/json", content);
      }
      

    });
  }
  }

/*******************************************************************
*    Implementation of itoa()  to conver  3 lsb of mac to ssid end, bytes are reversed
*    OASIS123456
*    0123456789a  index
*    acsii_mac   3C:61:05:0C:85:A0
*    index       00000000001111111
*     index      01234567890123456
* thanks to https://stackoverflow.com/users/168465/kriss
*******************************************************************/
void myitoa()
{
  int pointer = 10;
  int i = 0;
  int base = 16;
  int num;
  unsigned char * pin = mac1;
  const char * hex = "0123456789ABCDEF";
  char * pout = ascii_mac;
  for (; pin < mac1 + sizeof(mac1); pout += 3, pin++) {
    pout[0] = hex[(*pin >> 4) & 0xF];
    pout[1] = hex[ *pin     & 0xF];
    pout[2] = ':';
  }
  pout[-1] = 0;

  Serial.println(ascii_mac); //printf("%s\n", str);

  newssid[4] = ascii_mac[9];
  newssid[5] = ascii_mac[10];
  newssid[6] = ascii_mac[12];
  newssid[7] = ascii_mac[13];
  newssid[8] = ascii_mac[15];
  newssid[9] = ascii_mac[16];


}

/*******************************************************************
*   tbd
*   Functions called  to init adc1
*   also called every time there is a change
*   of config from a UDP message
********************************************************************/

void config_ADC(char chan, char att, char average, int msec)
{
  adcAttachPin(chan);
  // convert att to ADC_xxdb
  switch (att) {
    case 0:
      analogSetPinAttenuation(chan, ADC_0db);
      break;
    case 1:
      analogSetPinAttenuation(chan, ADC_2_5db);
      break;
    case 2:
      analogSetPinAttenuation(chan, ADC_6db);
      break;
    case 3:
      analogSetPinAttenuation(chan, ADC_11db);
      break;
  }
}

/**********************************************************************
*    Functions called when
*    the number of msec has expired, they will reset  counting, sum and
*    results, sampling will resume in Main
************************************************************************/
// blink led
void blinkLED()
{
  if (LED_state == 0)
  {
    digitalWrite(GPIO_LED, HIGH);
    LED_state = 1;
  } else {
    digitalWrite(GPIO_LED, LOW);
    LED_state = 0;
  }

}


/********************************************************************
*    ADC2 is different, conversions when wifi is off
*    may have to reset ESP32 so no possibility of doing average
*    average must be done by host,
*    conversion factor to mv must be meassured and saved on host
*
**********************************************************************/


void ADC2_0()
{
  /*  int read_raw;
     adc2_config_channel_atten( ADC2_CHANNEL_0, ADC_ATTEN_0db );
     esp_err_t r = adc2_get_raw( ADC2_CHANNEL_0, ADC_WIDTH_12Bit, &read_raw);
     if ( r == ESP_OK ) {
         printf("%d\n", read_raw );
     } else if ( r == ESP_ERR_TIMEOUT ) {
         printf("ADC2 used by Wi-Fi.\n");
         chan_avrgResults[5] =read_raw;  // non filtered results
     }
  */
}
/*
   adc attenuation esp32-c3
     ADC Att    ADC  Full Scale Reference Voltage
  0    0     100 - 750 mV
  1    2.5   100 - 1050
  2    6     100 - 1300
  3   11     100 - 2500


  Voltage input on ADC 0,1,2 for test 3x 27K in series as voltage divider
  1.2477V  ch0
  0.8300V  ch1
  0.4155V  ch2



  #include <driver/adc.h>
  #include <esp32-hal-adc.h>
  //#include <WiFi.h>
  //#include <BluetoothSerial.h>
  //#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
  //#define TIME_TO_SLEEP  5        // Time ESP32 will go to sleep (in seconds)

  //RTC_DATA_ATTR int bootCount = 0;

  long sumadc0,sumadc1,sumadc2;
  float avrg_adc0, avrg_adc1,avrg_adc2;
  int count=0;
  void setup() {
  //
  Serial.begin(115200);
  Serial.println("==================");
  adcAttachPin(0);
  adcAttachPin(1);
  adcAttachPin(2);
  //adc1_config_width(ADC_WIDTH_BIT_12);
  //adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  // adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11);
  // adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN_DB_11);

  analogSetPinAttenuation(0, ADC_11db);
  analogSetPinAttenuation(1, ADC_11db);
  analogSetPinAttenuation(2, ADC_11db);


  // sleep
  // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  // Setup LEDs for no output  8 is active low, 10 is active high
  pinMode(8, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(8, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(10, LOW);    // turn the LED off by making the voltage LOW

  }

  void loop() {

  int analogData0  = analogReadMilliVolts(0);
  int analogData1  = analogReadMilliVolts(1);
  int analogData2  = analogReadMilliVolts(2);
  if (count < 32){
  sumadc0+=analogData0;
  sumadc1+=analogData1;
  sumadc2+= analogData2;
  count++;
  }else{
  avrg_adc0 =sumadc0/32;
  avrg_adc1 =sumadc1/32;
  avrg_adc2 =sumadc2/32;

  Serial.print(avrg_adc0);
  Serial.print("    ");
  Serial.print(avrg_adc1);
  Serial.print("    ");
  Serial.println(avrg_adc2);
  Serial.println("==================");
  sumadc0=0;
  sumadc1=0;
  sumadc2= 0;
  count =0;


  }
  delay(100);
  //while(1);


  }
*/

/*
     add library to enable function to work
  #include<NoDelay.h>

  void ledBlink();//Must declare function before noDelay, function can not take arguments

  noDelay LEDtime(1000, ledBlink);//Creats a noDelay varible set to 1000ms, will call ledBlink function
  int LEDpin = 13;
  int ledState = LOW;

  void setup() {
  pinMode(LEDpin, OUTPUT);
  }

  void loop() {
    LEDtime.update();//will check if set time has past and if so will run set function
  }

  void ledBlink()
  {
  // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(LEDpin, ledState);
  }
*/
/*


  Startup

  SSID:Seenov21_2_4                    
  PASSWORD:bandittaichi                                                    
  ncncncConnected to Seenov21_2_4                     Successfully
  ESP Board MAC Address:  3C:61:05:0C:85:A0
  Local IP: 172.16.1.71
  SoftAP IP: 192.168.4.1
  end of setup
  loop
  UDP Packet Type: Unicast, From: 172.16.0.82:5044, To: 172.16.1.71:5044, Length: 6, Data: Master
  UDP Packet Type: Unicast, From: 172.16.0.82:5044, To: 172.16.1.71:5044, Length: 6, Data: Master
  UDP Packet Type: Broadcast, From: 172.16.0.82:5044, To: 172.16.1.255:5044, Length: 6, Data: Master
  UDP Packet Type: Broadcast, From: 172.16.0.82:5044, To: 172.16.1.255:5044, Length: 6, Data: Master
  UDP Packet Type: Broadcast, From: 172.16.0.82:5044, To: 172.16.1.255:5044, Length: 6, Data: Master
  UDP Packet Type: Broadcast, From: 172.16.0.82:5044, To: 172.16.1.255:5044, Length: 6, Data: Master
*/
