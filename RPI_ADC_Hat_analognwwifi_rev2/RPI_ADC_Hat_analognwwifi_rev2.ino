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

/*Program & Source Code:  6 Channel Raspberry Pi ADC with ESP32C3
  The Program is designed to follow the below steps.

  On First boot, the ESP32C3, in Station mode, will read the pre-registered SSID and password combination and try to connect to the same.

  If this process fails, it sets the ESP32C3 into AccessPoint mode and creates an open WiFi(Not protected with Password);

  This open WiFi will allow the user to connect Using any Wi-Fi enabled device with a browser. the SSID is ADC_xxyyzz xx-zz last 3 mac adress digits.

  After establishing a connection with the AccessPoint, you can go to the default IP address 192.168.4.1 to open a web page that allows you to input the SSID and password of your network;

  Once a new SSID and password is programmed into the EESP32C3's eeprom, the ESP32C3 reboots and tries to connect to your network;

  If it establishes a connection, the process is completed successfully. 

  If the connection fails again, it will restart the process.

  Once connected it waits for UDP messages from a Raspberry Pi running Node-Red. The UDP ports used are: send port 5045   receive on port 5044. 
  
  In Node-Red a broadcast UDP message is sent. The EESP32C3 ADC responds with its IP address enabling Node-Red to establish a one-on-one communication channel.

  The  ESP32C3 ADC and Node-Red exchange json encoded messages. 
   
Json messages from Node-Red
init Message
device1={type:"ADC1",ip:"0.0.0.0",name:"new module",category:"init",ch0:0,ch1:0,ch2:0,ch3:0,ch4:0,ch5:0,out1:0,out2:0,out3:0,link:"down"};

settings Message
{type: "ADC1",
name: "Mydev",
category: "setting",
ench0: 100,
ench1: 100,
ench2: 100,
ench3: 100,
ench4: 100,
ench5: 99,
delay: 100,
ota: 10,
dac: 0,
led2: 0,
io7: 1,
io18: 1,
io19: 1}


The ESP32C3 Raspberry Pi 6 Channel ADC has an Over-The_Air function built in. In response to a message from Node-Red the ESP32C3 
will again turn itself into an AccessPoint. You can login and select the new firmware .bin file to upload to the ESP32C3 ADC. 
Make sure you include this functionality in any new code or you will have to used a USB to serial adaptor to reprogram the ESP32C3 ADC.

Adding self test
Revision 2.0
   
*/


/*************************************************************
 *  for info
 *  /* Definitions for error constants. 
#define ESP_OK          0       /*!< esp_err_t value indicating success (no error)
#define ESP_FAIL        -1      /*!< Generic esp_err_t code indicating failure 
#define ESP_ERR_INVALID_ARG         0x102   /*!< Invalid argument 
#define ESP_ERR_INVALID_STATE       0x103   /*!< Invalid state 
#define ESP_ERR_INVALID_SIZE        0x104   /*!< Invalid size 
#define ESP_ERR_NOT_FOUND           0x105   /*!< Requested resource not found 
#define ESP_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported 
#define ESP_ERR_TIMEOUT             0x107   /*!< Operation timed out
#define ESP_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid
 * 
 **************************************************************/
#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include "AsyncUDP.h"
//#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include "ArduinoJson.h"
//#include <NoDelay.h>
#include <Update.h>
#include <esp32-hal-adc.h>
#include <Arduino.h>
#include "esp_adc_cal.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc_common.h"
#include <Update.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc_common.h"
//=================================

//Variables
uint8_t i = 0;

uint8_t LED_state = 1;
uint8_t AccesPointMode = 0;
uint8_t testBufffer[8] = {1, 1, 1, 1, 1, 1, 1, 1};
char channel_number;
int chan_enable[6]= {1,1,1,1,1,1};
char NumOfSamples_per_Avrg[6]={16,16,16,16,16,16};
int  msBetweenSamples[6]={50,0,0,0,0,0};
int  chan_att[6];
int  chan_coversionResults[6];
int adc_raw[6]={0,0,0,0,0,0};
int  chan_avrgResults[6]={0,0,0,0,0,0};
char counting[6] = {0, 0, 0, 0, 0, 0,};
const int test_avrg_results_high[6]={1038,827,623,417,208,802};
const int test_avrg_results_low[6]={1020,812,605,400,193,725};
double  chan_summ[6];
char channelToRead = 0;
char ADC2_ch0_samples=0;
char do_once=0;
int statusCode;
int ij=0;
int DAC_setting=0;
const char* ssid = "ASUS24";
const char* password = "Seenovtest";
String localwifissid= "ASUS24"; // for test
String localwifipsw= "Seenovtest";
String st;
String content;
String esid;
String hostname = "esp32c3_adc";
const char* host = "esp32c3_adc";
String epass = "";
char rebootNow = 0;
String selfTest ="";
char iotest1=0;
char iotest2=0;
char iotest3=0;
char iotest4=0;
int selfTestResults=0;
char softReboot = 0;
byte mac1[6];
char newssid[] = "ADC_123456";
char UDP_ch_active =0;
char ascii_mac[19] = {};
char temp_string[255] = {};
String testmac = "3c:61:05:0c:85:a0";
IPAddress ReceivedFromIP;
IPAddress ip;

/**************************************
 * Decremented in interval 3 20000 = 2000 sec.
 *  Turns off Serial.Print where not needed
 ***************************************/
int verbosePrint= 40000;
     
#define INTERVAL_1 5000
#define INTERVAL_2 3000   // adc2_0  low priority slow signal 1 conversions every xxxx mSec
#define INTERVAL_3 100    // blinking 100 mSec, decrements verbose
#define INTERVAL_4 3000
#define GPIO_LED   8
#define GPIO_LED2  10
#define DAC        6
#define GPIO_7     7
#define GPIO_18   18
#define GPIO_19   19
int LED2_low_high = 0;
int GPIO_7_value=0;
int GPIO_18_value=0;
int GPIO_19_value=0;
//ADC Attenuation
#define ADC_ATTEN           ADC_ATTEN_DB_11
//ADC Calibration
#define ADC_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_TP
#define OTA_Key 1234     // key to start OTA firmware update
int OTA=0;             // value read from UDP message
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

WebServer server(80);   // used to input network credentials and required for OTA Firmware update
AsyncUDP udp;

//====================================
/*
 * Login page for OTA required for OTA Firmware update
 */

const char* loginIndex =
 "<!DOCTYPE html> <html>  <head><meta name=  'viewport  ' content=  'width=device-width, initial-scale=1.0, user-scalable=no  '>  "
 "<title>ESP WiFi Manager</title>   <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
 "body{margin-top: 50px;} h1 {color: #444444;margin: 30px auto 30px;}   h3 {color: #ffffff;margin-bottom: 50px;}   label{display:inline-block;width: 160px;text-align: center;font-size: 1.5rem; font-weight: bold;color: #ffffff;}"
"form{margin: 0 auto;width: 360px;padding: 1em;border: 1px solid #CCC;border-radius: 1em; background-color: #077c9f;}    input {margin: 0.5em;font-size: 1.5rem; } "
 " .styled {    border: 0;    line-height: 2.5;    padding: 0 20px;    font-size: 1.5rem;    text-align: center;    color: #fff;    text-shadow: 1px 1px 1px #000; "
 "  border-radius: 10px;    background-color: rgba(220, 0, 0, 1);    background-image: linear-gradient(to top left,  rgba(0, 0, 0, .2), rgba(0, 0, 0, .2) 30%, "
 "  rgba(0, 0, 0, 0));    box-shadow: inset 2px 2px 3px rgba(255, 255, 255, .6),   inset -2px -2px 3px rgba(0, 0, 0, .6);.styled:hover {   background-color: rgba(255, 0, 0, 1);} "
".styled:active {    box-shadow: inset -2px -2px 3px rgba(255, 255, 255, .6), inset 2px 2px 3px rgba(0, 0, 0, .6);}  </style>"  
" <meta charset=  'UTF-8  '>    </head>   <body>   <h1>Welcome to ADC1 Firmware Update</h1>    <h1>Login Page</h1> " 
"   <form name = 'loginForm' method='get' action='setting'> <div><label>Username</label> <input type ='text' name= 'userid' length= 32></div>"  
"  <div><label>Password </label> <input type = 'Password' name=  'pwd'  length= 32></div>  "
" <h3>Please Double Check Before Saving</h3>   "
"<button class= 'favorite styled ' type= 'submit' onclick='check(this.form)' value='Login'> SUBMIT</button> </form> " 

"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>"
" </body>  </html> ";


/*
 * Server Index Page for OTA required for OTA Firmware update
 */

const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

static esp_adc_cal_characteristics_t adc1_chars;
static esp_adc_cal_characteristics_t adc2_chars;

static bool adc_calibration_init(void)
{
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ADC_CALI_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        Serial.println( "Calibration scheme not supported, skip software calibration");
    } else if (ret == ESP_ERR_INVALID_VERSION) {
        Serial.println( "eFuse not burnt, skip software calibration");
    } else if (ret == ESP_OK) {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH_BIT_12, 0, &adc1_chars);
        esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN, ADC_WIDTH_BIT_12, 0, &adc2_chars);
    } else {
        Serial.println("Invalid arg");
    }

    return cali_enable;
}
 bool cali_enable = adc_calibration_init();
//=======================================

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
  pinMode(GPIO_LED2, OUTPUT);
  digitalWrite(GPIO_LED2, LOW);   // off
  pinMode(DAC, OUTPUT);
  pinMode(GPIO_7, OUTPUT);  // can be redefined as inputs
  pinMode(GPIO_18, OUTPUT); // can be redefined as inputs
  pinMode(GPIO_19, OUTPUT); // can be redefined as inputs
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
  //esid="test";  // Used for testing to force bad ssid and psw forcing AccessPoint mode
  Serial.println();
  Serial.print("SSID:");
  Serial.println(esid);


  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
   
  Serial.print("PASSWORD:");
  Serial.println(epass);
  /********************************************
   *    Check if selftest done 
   *   if not "T" do selftest
   *   
   ********************************************/
  selfTest += char(EEPROM.read(96));  // to force self test:  selfTest = "X";
  if(selfTest != "T"){   
     Serial.println("LED and GPIO test");
    // test LEDs  user vissual test
    for(char xx=0;xx<15;xx++){
      
    digitalWrite(GPIO_LED, LOW);   // on
    digitalWrite(GPIO_LED2, LOW);   // off
    delay(200);
    digitalWrite(GPIO_LED, HIGH);   // off
    digitalWrite(GPIO_LED2, HIGH);   // on
    delay(200);
    }
    
    // test GPIO
    pinMode(GPIO_7, OUTPUT);  // can be redefined as inputs
    pinMode(GPIO_18, INPUT); // can be redefined as inputs
    pinMode(GPIO_19, INPUT); // can be redefined as inputs   
    digitalWrite(GPIO_7, HIGH);
    iotest1 = digitalRead(GPIO_18);
    iotest2 = digitalRead(GPIO_19);
    Serial.println("test gpio 7 18 and 19 high");
    delay(100);
    digitalWrite(GPIO_7, LOW);
    iotest3 = digitalRead(GPIO_18);
    iotest4 =digitalRead(GPIO_19);
    Serial.println("test gpio 7 18 and 19 low");
    Serial.println(iotest1,DEC);
    Serial.println(iotest2,DEC);
    Serial.println(iotest3,DEC);
    Serial.println(iotest4,DEC);
    delay(100);
    
    if( (iotest1==0)  ){
      selfTestResults += 1;
      Serial.print("Error GPIO18 or 7 ");
    }
     if(  (iotest3==1) ){
      selfTestResults += 2;
      Serial.print("Error GPIO18 or 7");
    }
    if( (iotest2==0) ){
      selfTestResults += 4;
      Serial.print("Error GPIO19 or 7");
    }
    if( (iotest4==1)){
      selfTestResults += 8;
      Serial.print("Error GPIO18 or 7");
    }
    
    // set back to normal
    pinMode(GPIO_7, OUTPUT);  // can be redefined as inputs
    pinMode(GPIO_18, OUTPUT); // can be redefined as inputs
    pinMode(GPIO_19, OUTPUT); // can be redefined as inputs    
    digitalWrite(GPIO_7, LOW);
    digitalWrite(GPIO_18, LOW);
    digitalWrite(GPIO_19, LOW);
    delay(100);
    // setup ADC
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN));
    //ADC2 config
    ESP_ERROR_CHECK(adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN));

    // Test DAC and ADC2 ch 0  resistor divider between DAC out and ADC2_ch0
    analogWrite(DAC, 120);
    Serial.println("");
    Serial.println("Test DAC and ADC2  ");
    delay(100);
    int tempadc2=0;
            
    esp_err_t ret1 = ESP_OK;
       
          for( char jj=0; jj< NumOfSamples_per_Avrg[5]; jj++)
          {
              delay(100);
              ret1 = adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &adc_raw[5]); 
                  
               delay(100);
              if ( (cali_enable) &&  (ret1 == ESP_OK))
              {
                  tempadc2=esp_adc_cal_raw_to_voltage(adc_raw[5], &adc2_chars);
                  chan_summ[5]+=  tempadc2;
               }
         }
         chan_avrgResults[5]=  chan_summ[5]/NumOfSamples_per_Avrg[5];
         Serial.print(" ADC2  = ");
         Serial.println(chan_avrgResults[5],DEC);
         
         if( (chan_avrgResults[5] < test_avrg_results_low[5] ) ||  (chan_avrgResults[5]> test_avrg_results_high[5] ) ) // replace xxx yyy with boundry
         {
                selfTestResults += (16);
                Serial.println(" ADC2 Error ");
         } 
         delay(500); 
    // reset to zero
    chan_summ[5]=0;
    analogWrite(DAC, 0);  
    ADC2_ch0_samples=0;
    chan_avrgResults[5]=0;
     
  
    //Test reference and ADC 1,2,3,4  ch_0
    // resistor divider between ref out and ADC
       for (char samples1=0; samples1<NumOfSamples_per_Avrg[0]; samples1++)
      {
          for(char ii=0; ii<5;ii++)
          {
            switch (ii)
            {
             case 0:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_0);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
                      break;
             case 1:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_1);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
                      break;
             case 2:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_2);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
                      break;
             case 3:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_3);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
                      break;
             case 4:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_4);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
             }
              
          }
      }
      for(char ii=0; ii<5;ii++)
      {
               chan_avrgResults[ii]= chan_summ[ii]/NumOfSamples_per_Avrg[0];
               
               if( (chan_avrgResults[ii]< test_avrg_results_low[ii] ) ||  (chan_avrgResults[ii]> test_avrg_results_high[ii] ) ) // replace xxx yyy with boundry
         {
                selfTestResults += (16 + 2^ii);
                Serial.print("*****Error ADC1 chan No =  ");
                Serial.println(ii,DEC);
         }  
               chan_summ[ii]=0;
               Serial.print("  Chan = ");
               Serial.print(ii, DEC);
               Serial.print("  ");
               Serial.println(chan_avrgResults[ii]);
               chan_avrgResults[ii] =0;
      }
      Serial.print( "    Self Test Results (0= No issues) =  ");
      Serial.println(selfTestResults, DEC);
       for(char xx=0;xx<5;xx++){
      
    digitalWrite(GPIO_LED, LOW);   // on
    digitalWrite(GPIO_LED2, LOW);   // off
    delay(200);
    digitalWrite(GPIO_LED, HIGH);   // off
    digitalWrite(GPIO_LED2, HIGH);   // on
    delay(200);
    }
    digitalWrite(GPIO_LED, HIGH);   // off
    digitalWrite(GPIO_LED2, LOW);   // off
      // if no issue  programm eeprom else restart
      if(selfTestResults == 0)
      {
            selfTest = "T";  // change later128
            Serial.println("writing eeprom self test ok");
            EEPROM.write(96, 0);
            EEPROM.write(96, selfTest[0]);
            EEPROM.commit();
            delay(2000);
            
      } else {
         ESP.restart();      
      }
  }// end of self test
  
  /******************************************************
   * 
   * Try to connect to local WiFi with saved ssid psw
   * 
   ***************************************************/
  WiFi.begin(esid.c_str(), epass.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac1); // returns mac address
  
  char temp = 10;    // seconds tying to connect

  //  wait up to 10 sec  for connection
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
 *   Successful connexion, 
 *   check UDP  messages
 ******************************************/

    if((udp.listen(5044)) && ( AccesPointMode==0)) {
    Serial.print("UDP Listening  ");
    udp.onPacket([](AsyncUDPPacket packet) {
        //Serial.print("UDP Packet Type: ");
        //Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
        //Serial.print(", From: ");
        ReceivedFromIP = packet.remoteIP();  // needed for reply
        //Serial.print(packet.remoteIP());
        //Serial.print(":");
        //Serial.print(packet.remotePort());
        //Serial.print(", To: ");
        //Serial.print(packet.localIP());
        //Serial.print(":");
        //Serial.print(packet.localPort());
        if(verbosePrint) 
        {
         Serial.print(", Length: ");
         Serial.print(packet.length());
         Serial.println(", Data: ");
         Serial.write(packet.data(), packet.length());
         Serial.println();
         packet.printf("Got %u bytes of data", packet.length());
        }
        //parse json msg  after copy to buff
        char tempBuff[packet.length()+10];
        memcpy(tempBuff, packet.data(), packet.length());
        DeserializationError error = deserializeJson(doc_r, tempBuff);
        if (error) {
          Serial.print(F("deserializeJson() failed with code "));
          Serial.println(error.f_str());
          
        }else {               // no error use the values received
    /****************************************************
     *         Decode received messages             
     *         tbd add  ADC identifyer so multiple units
     *         can communicate with Raspberry Pi
    ****************************************************/
         const char* type1 = doc_r["type"];
         const char* ADC = "ADC1";
         const char* category = doc_r["category"];
         const char* settingstype = "setting";
         const char* init_type = "init";
         if (verbosePrint){Serial.println(category);}   // setting or init
         if (strcmp(category, init_type)==0 )
        {
            Serial.println("Received Connection Init ");
            //Serial.print(", From: ");
            //Serial.print(packet.remoteIP());
            Serial.println(type1);
            if (strcmp(type1,  ADC)==0 )
            {
                Serial.println("UDP Channel open");
                UDP_ch_active=1;
            }
        }
   
         if (strcmp(category,  settingstype)==0 )
        {
           if(verbosePrint){ Serial.println("received settings");}
           // delay between sample bursts
           msBetweenSamples[0]=(int)doc_r["delay"];  //only one delay for all channels
            
            // Enable / disable channels 
            chan_enable[0] = (int)doc_r["ench0"]-99;
            chan_enable[1] = (int)doc_r["ench1"]-99;
            chan_enable[2] = (int)doc_r["ench2"]-99;
            chan_enable[3] = (int)doc_r["ench3"]-99;
            chan_enable[4] = (int)doc_r["ench4"]-99;
            chan_enable[5] = (int)doc_r["ench5"]-99;
           
            //  start Over the air firmare update
            OTA = (int)doc_r["ota"];
            if(verbosePrint){
            Serial.print("OTA=  ");
            Serial.println(OTA);
            }
            // DAC Setting set DAC pwm value
            DAC_setting = (int)doc_r["dac"];
            analogWrite(DAC, DAC_setting);
            
            //LED2  active high
            LED2_low_high = (int)doc_r["led2"];
            digitalWrite(GPIO_LED2, LED2_low_high); 
            
            // Digital outputs
            GPIO_7_value=(int)doc_r["io7"];
            digitalWrite(GPIO_7, GPIO_7_value); 
            GPIO_18_value=(int)doc_r["io18"];
            digitalWrite(GPIO_18, GPIO_18_value);
            GPIO_19_value= (int)doc_r["io19"];
            digitalWrite(GPIO_19, GPIO_19_value);


       /****************************************************
        *  The attenuation is fixed at 11 db
        *  Set attenuation with Node-Red TBD
        *     
        * chan_att[0] = (int) doc_r[" xxx0"];     
        * chan_att[1] = (int) doc_r[" xxx1"];
        *   etc for remaining channels
        *
        *******************************************************/    
            
        }
        
        }
          
      });
      
    }
  
  // Print usefull information for debugging
  String mac2 = (String)WiFi.macAddress() ;
  Serial.print("ADC Board MAC Address:  ");
  Serial.println(mac2);
  if(AccesPointMode ==0)
  {
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
  }
  
   
  
 /**********************************************************************
  *   
  *     ADC1&2 config fixed bit width and fixed attenuation
  *    tbd   configure attenuation from Node-Red messages 
  *    
  ************************************************************************/
   
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN));
    //ADC2 config
    ESP_ERROR_CHECK(adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN));
 }


void loop() {

//required for OTA Firmware update
 if ((AccesPointMode == 1) || (OTA == OTA_Key)) 
  {
   
   server.handleClient();
   
  }

 
  /****************************************
   * tbd
   * Function to force get new chredentials by software
   * Add Node-red object  that will be decoded 
   * 
   ***************************************
  if (softReboot == 1)
  {
    softReboot = 0;
    Serial.println("Turning the HotSpot On");
    setupAP();// Setup HotSpot to get new SSID and PSW
    AccesPointMode = 1;
    digitalWrite(GPIO_LED, LOW);
  }
  */
  
   /**********************************************
   *  required for OTA Firmware update
   * Function for over the air programming
   * In Node-Red push OTA button 
   * Go to web page  http://esp32c3_adc.local 
   * login user: admin psw: admin
   * 
   ***********************************************/
      if((OTA == OTA_Key) &&(do_once==0))
      {
           do_once=1;
             //use mdns for host name resolution
              if (!MDNS.begin(host)) { //http://esp32c3_adc.local
                Serial.println("Error setting up MDNS responder!");
                while (1) {
                  Serial.print("+");
                  delay(1000);
                }
              }
              server.begin();
              Serial.println("mDNS responder started");
              //return index page which is stored in serverIndex 
              server.on("/", HTTP_GET, []() {
                server.sendHeader("Connection", "close");
                server.send(200, "text/html", loginIndex);
              });
              server.on("/serverIndex", HTTP_GET, []() {
                server.sendHeader("Connection", "close");
                server.send(200, "text/html", serverIndex);
              });
              //handling uploading firmware file 
              server.on("/update", HTTP_POST, []() {
                server.sendHeader("Connection", "close");
                server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
                ESP.restart();
              }, []() {
                HTTPUpload& upload = server.upload();
                if (upload.status == UPLOAD_FILE_START) {
                  Serial.printf("Update: %s\n", upload.filename.c_str());
                  if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                    Update.printError(Serial);
                  }
                } else if (upload.status == UPLOAD_FILE_WRITE) {
                  // flashing firmware to ESP
                  if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    Update.printError(Serial);
                  }
                } else if (upload.status == UPLOAD_FILE_END) {
                  if (Update.end(true)) { //true to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                  } else {
                    Update.printError(Serial);
                  }
                }
              });
              
            
      }

     
  
    /*****************************************************************************
    * 
    *    Create and Send udp json packet with ADC values in int32 mV format
    *    samples are averages of 16 samples  
    *    Important no output until receive from udp to find ip of destination send to
    *     json doc doc_s send. Stop if OTA is active
    *    Node-Red extracts the ip from the first of these messages
    *  
    ************************************************************************************/
    if((millis() >= time_4 + INTERVAL_4)&& (OTA != OTA_Key))
    {
        time_4 += INTERVAL_4;
        // always send all 6 channels
        doc_s["type"] = "ADC1";  //  define packet content
        doc_s["ch0"] = chan_avrgResults[0];
        doc_s["ch1"] = chan_avrgResults[1];
        doc_s["ch2"] = chan_avrgResults[2];
        doc_s["ch3"] = chan_avrgResults[3];
        doc_s["ch4"] = chan_avrgResults[4];
        doc_s["ch5"] = chan_avrgResults[5];
        // Send the data over UDP if  active
        if(UDP_ch_active==1)
        {
            serializeJson(doc_s, fromString);
            SendLen = strlen( fromString );
            memcpy( udpString, fromString, SendLen );
            if(verbosePrint)
            {
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
            Serial.print("    ch 5 mV: ");
            Serial.println(chan_avrgResults[5]); 
            }
            AsyncUDPMessage msg(strlen(fromString));
            msg.write( udpString, SendLen );
            udp.sendTo( msg, ReceivedFromIP, 5045 );
        }
   }

  

  /***************************************************************
   * 
   * Do conversions for active channels 0 to 4 (ADC 1)
   * Sum 16 conversions per enabled channel 
   * NumOfSamples_per_Avrg[0] sets average for all channels
   * Conversions will faster if less channels enabled
   * Calculate average after all samples are done
   * Stop if OTA active
   * 
   ***************************************************************/
    
    if((millis() >= time_1 + msBetweenSamples[0])&&(OTA != OTA_Key))
    {
      time_1 +=msBetweenSamples[0];
         
        
      for (char samples1=0; samples1<NumOfSamples_per_Avrg[0]; samples1++)
      {
          for(char ii=0; ii<5;ii++)
          {
            switch (ii)
            {
             case 0:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_0);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
                      break;
             case 1:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_1);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
                      break;
             case 2:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_2);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
                      break;
             case 3:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_3);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
                      break;
             case 4:  if (chan_enable[ii]==1)  // if active convert
                      {
                        adc_raw[ii] = adc1_get_raw(ADC1_CHANNEL_4);
                        chan_summ[ii]+=esp_adc_cal_raw_to_voltage(adc_raw[ii], &adc1_chars);
                      }
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
    /********************************************************
     *  Low priority ADC2 channel 0. May interfere with wifi.
     *  Do one sample on each pass at 16th sample calculate 
     *  average, if the calibration is not enabled or the 
     *  conversion results are not ESP_OK skip the sample
     *  stop during OTA
     *  it will take  16 x INTERVAL_2 before a value is ready
     ********************************************************/
      if((millis() >= time_2 + INTERVAL_2)&&(OTA != OTA_Key))
      {
        esp_err_t ret1 = ESP_OK;
        time_2 +=INTERVAL_2;
       
         //may need to add  WiFi.disconnect() before ADC2 call to avoid interference
         if (chan_enable[5]==1)  // if active convert
        {
          ret1 = adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &adc_raw[5]);
          if ( (cali_enable) &&  (ret1 == ESP_OK))
          {
              chan_summ[5]+=esp_adc_cal_raw_to_voltage(adc_raw[5], &adc2_chars);  // was adc1 wrong
              ADC2_ch0_samples++;
              if( ADC2_ch0_samples == NumOfSamples_per_Avrg[5])
              {
                  chan_avrgResults[5]= chan_summ[5]/NumOfSamples_per_Avrg[5];
                  chan_summ[5]=0;
                  ADC2_ch0_samples=0;
              }
          }
        }else{
          chan_avrgResults[5]= 0;
                  chan_summ[5]=0;
                  ADC2_ch0_samples=0; 
        }
         
             
     }

   /*******************************************
    * 
    *    For future use
    * 
    *******************************************/
    if(millis() >= time_3 + INTERVAL_3)
    {
        time_3 +=INTERVAL_3;
        //blinkLED();
        
        if(verbosePrint>0)
        {
          verbosePrint--;
        }
        
        
   }

    
   
    
}

/*******************************************************************
* 
*     Fuctions used for WiFi credentials saving and connecting
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
*   Start the web server to get the new ssid and password 
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
   
  // Start the server
  server.begin();
  createWebServer();
  Serial.println("Server started");
  }

/*******************************************************************
*    
*    
*    Set up Access Point so user can connect as a client, access the 
*    web server and input ssid and password default IP 192.168.4.1
*    
*    
*******************************************************************/
void setupAP(void)
{
  
  WiFi.disconnect();
  delay(100);
  // scan for testing
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
      Serial.println(")");
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
 
   
  WiFi.macAddress(mac1);
  //   ****************  we set a new ssid ADC_ +  last 3 parts of mac **********
  //   
    myitoa(); // generates new SSID from mac address
   Serial.println(newssid);
   
  WiFi.softAP(newssid,"");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Connect to SSID ADC_xxxxx, no password required");
  Serial.println("Go to web page and update SSID and PASW for your network");

}

/*******************************************************************
*     
*     Create web server to input Local WiFi credentials
*    
*    
*******************************************************************/
  void createWebServer()
  {
  
    server.on("/", []() {
      
     // IPAddress ip = WiFi.softAPIP();
     // String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE html> <html>  <head><meta name=  'viewport  ' content=  'width=device-width, initial-scale=1.0, user-scalable=no  '>  ";
 content+= " <title>ESP WiFi Manager</title>   <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
 content+= " body{margin-top: 50px;} h1 {color: #444444;margin: 30px auto 30px;}   h3 {color: #ffffff;margin-bottom: 50px;}   label{display:inline-block;width: 160px;text-align: center;font-size: 1.5rem; font-weight: bold;color: #ffffff;}";
 content+= " form{margin: 0 auto;width: 360px;padding: 1em;border: 1px solid #CCC;border-radius: 1em; background-color: #077c9f;}    input {margin: 0.5em;font-size: 1.5rem; }  "; 
   content+= "   .styled {    border: 0;    line-height: 2.5;    padding: 0 20px;    font-size: 1.5rem;    text-align: center;    color: #fff;    text-shadow: 1px 1px 1px #000; ";
  content+= "  border-radius: 10px;    background-color: rgba(220, 0, 0, 1);    background-image: linear-gradient(to top left,  rgba(0, 0, 0, .2), rgba(0, 0, 0, .2) 30%, ";
  content+= "   rgba(0, 0, 0, 0));    box-shadow: inset 2px 2px 3px rgba(255, 255, 255, .6),   inset -2px -2px 3px rgba(0, 0, 0, .6);.styled:hover {   background-color: rgba(255, 0, 0, 1);} ";
content+= ".styled:active {    box-shadow: inset -2px -2px 3px rgba(255, 255, 255, .6), inset 2px 2px 3px rgba(0, 0, 0, .6);}  </style>  "; 
content+= "  <meta charset=  'UTF-8  '>    </head>   <body>   <h1>Welcome to WiFi Update</h1>    <h1>ESP32C3 6 Channel ADC</h1> ";  
content+= "       <h1>Enter local WiFi SSID&PSW</h1>    <form method='get' action='setting'> <div><label>WiFi SSID</label> <input name= 'ssid' length= 32></div> ";  
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

  //Serial.println(ascii_mac); //printf("%s\n", str);

  newssid[4] = ascii_mac[9];
  newssid[5] = ascii_mac[10];
  newssid[6] = ascii_mac[12];
  newssid[7] = ascii_mac[13];
  newssid[8] = ascii_mac[15];
  newssid[9] = ascii_mac[16];


}



/**********************************************************************
*    Blink LED
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
