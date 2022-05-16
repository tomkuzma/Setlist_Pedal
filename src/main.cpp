/*
Setlist Pedal Prototype
- Reads user-selected text file from SD card and scrolls through with buttons
- Upload and update files over wifi
- Connects to NTP server to update and display time, otherwise uses RTC module

Author: Tom Kuzma
Date: April 28, 2022
 */

/////// SD CARD PINS /////    //////// RTC PINS //////////
//  SD Card |  ESP32    //    //   DS3231   |   ESP32   //  
//////////////////////////    ////////////////////////////
// VCC      |   5V      //    //   VCC      |   5V      //               
// GND      |   GND     //    //   GND      |   GND     //                
//  SCK     |  GPIO 18  //    //   SCL      |   GPIO 22 //                
//  MISO    |  GPIO 19  //    //   SDA      |   GPIO 21 //                 
//  MOSI    |  GPIO 23  //    ////////////////////////////                   
//  CS      |  GPIO 5   //    
//////////////////////////    

#include <Arduino.h>
#include "WiFi.h"
#include "time.h"
#include "RTClib.h"
// #include <Wire.h>
#include "SDCard.h"
#include "DailyStruggleButton.h"
#include "SPI.h"
#include <TFT_eSPI.h>
#include "Free_Fonts.h"

#define LEFT_BUTTON_PIN     16
#define RIGHT_BUTTON_PIN    17
#define PRESS_TIME          1000
#define DEBOUNCE_TIME       20  
#define RTC_PERIOD          5000

// prototypes
void buttonEvent_left (byte btnStatus);
void buttonEvent_right (byte btnStatus);
void initWiFi();
void syncTime();

// FSM states
enum State {UPDATE_NTP, SERVER_CONNECT, SELECT_FILE, SCROLL_FILE};
State STATE = UPDATE_NTP;

// for non blocking loop actions
unsigned long lastTime = 0;

// Globals for indexing file text lines
int lineCount;
int startingLine = 0;

// Button Globals 
DailyStruggleButton lefftButton;
DailyStruggleButton rightButton;

// rtc Global
RTC_DS3231 rtc;
DateTime prev, now;

//WiFi Details
const char* ssid       = "TELUS6010";
const char* password   = "roti2roti";

// NTP server to request time
const char* ntpServer = "pool.ntp.org";

// TFT Object
TFT_eSPI tft = TFT_eSPI();

// for sending time to screen ** MAY NOT NEED TO BE GLOBAL
// String timeString = "";

////////////////SETUP////////////////////////////
void setup()
{
    Serial.begin(115200);

//////////////////////////////////////////////
//      RTC SETUP                           //
//////////////////////////////////////////////  
    // removed this because it was throwing Guru Meditation Error for unknown reason
    // Would reset time if battery died on RTC module 
    // if (rtc.lostPower()) {
    // // reset time to compile timestamp if power lost
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // }

//////////////////////////////////////////////
//      WIFI SETUP                          //
////////////////////////////////////////////// 
    rtc.begin(); //Start RCT

    //Wifi
    initWiFi();

    //Time Server
    configTime(0, 0, ntpServer);

    // sync RTC time with NTP server
    syncTime();
    prev = rtc.now();
    now = rtc.now();

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

//////////////////////////////////////////////
//       SD CARD SETUP                      //
//////////////////////////////////////////////

    // Mount SD card
    if (!SD.begin())
    {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC){
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD){
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC){
        Serial.println("SDHC");
    }
    else{
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    listDir(SD, "/", 0);

    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

    lineCount = scanFile(SD, "/test.txt");
    readLines(SD, startingLine, lineCount, "/test.txt");

//////////////////////////////////////////////
//     BUTTONS SETUP                        //
//////////////////////////////////////////////
    lefftButton.set(LEFT_BUTTON_PIN, buttonEvent_left, INT_PULL_UP);
    lefftButton.setDebounceTime(DEBOUNCE_TIME);
    lefftButton.enableLongPress(PRESS_TIME);

    rightButton.set(RIGHT_BUTTON_PIN, buttonEvent_right, INT_PULL_UP);
    rightButton.setDebounceTime(DEBOUNCE_TIME);
    rightButton.enableLongPress(PRESS_TIME);

//////////////////////////////////////////////
//     TFT SETUP                            //
//////////////////////////////////////////////
  tft.init();
  tft.setRotation(3); 

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0,120,2);
  tft.setFreeFont(FSSB18);
  tft.println("- Hello World!!");
  tft.println("- Testing song with long songname here");
  tft.println("- Another Song");

} // end setup

void loop()
{
    // button polling
    lefftButton.poll();
    rightButton.poll();

    if (millis() - lastTime >= RTC_PERIOD) {
        now = rtc.now();   // get current time from rtc module
        lastTime = millis();

        // update time string if second changes
        if (now.minute() != prev.minute()) {
            char buff[] = ":mm:ss AP";  // for time format display
            prev = now;
            String timeString = String(now.twelveHour()) + now.toString(buff); // make nice looking 12 hour time string with no leading zeros
            Serial.println(timeString);
        }
    }

} // end loop

// 
void buttonEvent_left (byte btnStatus){
    switch (btnStatus)
    {
    case onPress:
        if (startingLine != 0) {
            readLines(SD, --startingLine, lineCount, "/test.txt"); // decrement starting line and get 3 lines
        }
        break;

    case onLongPress:
        // do nothing currently
    break;
    
    default:
        break;
    }
} // end buttonEvent_left

void buttonEvent_right (byte btnStatus){
    switch (btnStatus)
    {
    case onPress:
        if (startingLine < lineCount - 2){
            readLines(SD, ++startingLine, lineCount, "/test.txt"); // increment starting line and get 3 lines
        }
        break;

    case onLongPress:
        // lineCount = scanFile(SD, "/test.txt");
        Serial.println(listDir(SD, "/", 0));
    break;
    
    default:
        break;
    }
} // end buttonEvent_right

// Initialize WiFi
void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi ..");

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }

    Serial.println(WiFi.localIP());
}

// gets time from NTP Server and syncs RTC clock
void syncTime() {

    struct tm NTP;

    // set timezone and DST for Vancouver *** MAY HAVE TO MAKE MENU FOR USER SELECTED TIMEZONE FOR FUTURE UPDATE ***
    setenv("TZ","PST8PDT,M3.2.0,M11.1.0",1);
    tzset();
    getLocalTime(&NTP);

    rtc.adjust(DateTime(NTP.tm_year + 1900, NTP.tm_mon + 1, NTP.tm_mday, NTP.tm_hour, NTP.tm_min, NTP.tm_sec));
    Serial.println("Time Synced to NTP Server!");

    // print time
    now = rtc.now();
    char buff[] = ":mm:ss AP";  // for time format display
    String timeString = String(now.twelveHour()) + now.toString(buff); // make nice looking 12 hour time string with no leading zeros
    Serial.println(timeString);

} // end syncTime