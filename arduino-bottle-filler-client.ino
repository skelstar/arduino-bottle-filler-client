#include <myWifiHelper.h>
#include <myPushButton.h>
#include <ArduinoJson.h>            // https://github.com/bblanchon/ArduinoJson
#include "LPD8806.h"
#include "SPI.h"
#include <SevenSegmentTM1637.h>
#include <SevenSegmentExtended.h>
#include <TaskScheduler.h>
#include <TimeLib.h>
#include <string.h>
#include <stdio.h>


#define     WIFI_HOSTNAME "home-bedroom-client"

#define     TOPIC_LIAMROOMTRIP          "/node/liamsroom/trip"
#define     TOPIC_TIMESTAMP             "/dev/timestamp"
#define     TOPIC_COMMAND               "/bedroom/bedside-client/command"
#define     TOPIC_BUTTONPUSHED          "/bedroom/bedside-client/button"
#define     TOPIC_ONLINE                "/bedroom/bedside-client/online"


#define     DISPLAY_TIME    '1'
#define     CLEAR_TIME      '2'
    
// #defi    ne MED_BRIGHT      30
#define     LOW_BRIGHT      10
#define     CLK             D4       // 0 (-01)
#define     DIO             D5       // 2 (-01)
#define     BUTTON          D2
#define     LPD8806_DATA    D0
#define     LPD8806_CLK     D1

/* ----------------------------------------------------------------------------- */

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

SevenSegmentExtended sevenSeg(CLK, DIO);
// http://playground.arduino.cc/Main/SevenSegmentLibrary

void button_callback(int eventCode, int eventParam);
myPushButton button(BUTTON, false, 3000, LOW, button_callback);


LPD8806 strip = LPD8806(2, LPD8806_DATA, LPD8806_CLK);

uint32_t    COLOUR_OFF = strip.Color(0, 0, 0);  
uint32_t    COLOUR_RED = strip.Color(10, 0, 0);

uint32_t  currentstripColor = COLOUR_OFF;
volatile uint32_t  flashStripColour = COLOUR_OFF;

Scheduler runner;

#define RUN_ONCE    2

void tCallback_FlashTriggerLEDON();
void tCallback_FlashTriggerLEDOFF();
Task tFlashMotionLED(50, 2, &tCallback_FlashTriggerLEDON, &runner, false);
void tCallback_FlashTriggerLEDON() {
    strip.setPixelColor(0, flashStripColour);
    strip.setPixelColor(1, flashStripColour);
    strip.show();
        Serial.println("FLASHON");
    tFlashMotionLED.setCallback(tCallback_FlashTriggerLEDOFF);
}
void tCallback_FlashTriggerLEDOFF() {
    strip.setPixelColor(0, currentstripColor);
    strip.setPixelColor(1, currentstripColor);
    strip.show();
        Serial.println("FLASHOFF");
    tFlashMotionLED.setCallback(tCallback_FlashTriggerLEDON);
}


/* ----------------------------------------------------------------------------- */

void button_callback(int eventCode, int eventParam) {

    switch (eventParam) {
        case button.EV_BUTTON_PRESSED:
            wifiHelper.mqttPublish(TOPIC_BUTTONPUSHED, "1");
            Serial.println("TOUCHED");
            break;
        case button.EV_RELEASED:
        case button.ST_WAITING_FOR_RELEASE_FROM_HELD_TIME:
            wifiHelper.mqttPublish(TOPIC_BUTTONPUSHED, "0");
            Serial.println("not touched");
            break;
        default:    
            Serial.println("some event");
            break;
    }
}

/* ----------------------------------------------------------------------------- */

void mqttcallback_Timestamp(byte *payload, unsigned int length) {
    unsigned long pctime = strtoul((char *)payload, NULL, 10);
    setTime(pctime);
    wifiHelper.mqttPublish(TOPIC_ONLINE, "1");
    //Serial.print("timestamp: "); Serial.print(hour()); Serial.println(minute());
}

void mqttcallback_DisplayTime(byte *payload, unsigned int length) {

    if (payload[0] == DISPLAY_TIME) {
        sevenSegDisplayTime();
    }
    else if (payload[0] == CLEAR_TIME) {
        sevenSegClear();
        //sevenSeg.print("----");
    }
    else {
        Serial.print("/node/liamsroom/trip: "); Serial.println(payload[0]);
    }
}

void mqttcallback_Command(byte *payload, unsigned int length) {

    StaticJsonBuffer<50> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(payload);

    if (!root.success()) {
        Serial.println("parseObject() failed");
    }

    const char* command = root["command"];
    const char* value = root["value"];

    // root.printTo(Serial);

    // Serial.print("command: "); Serial.println(command);
    // Serial.print("value: "); Serial.println(value);

    if (strcmp(command, "PIXEL") == 0) {

        const char d[2] = ",";
        char* colors = strtok((char*)value, d);
        char* red = colors;
        colors = strtok(NULL, d);
        char* blu = colors;         // R.B.G for LPD8806
        colors = strtok(NULL, d);
        char* grn = colors;

        uint8_t redi = atoi(red)/2;
        uint8_t grni = atoi(grn)/2;
        uint8_t blui = atoi(blu)/2;

        currentstripColor = strip.Color(redi, grni, blui);
        strip.setPixelColor(0, currentstripColor);
        strip.setPixelColor(1, currentstripColor);
        strip.show();
    }
    else if (strcmp(command, "FLASH") == 0) {

        const char d[2] = ",";
        char* colors = strtok((char*)value, d);
        char* red = colors;
        colors = strtok(NULL, d);
        char* blu = colors;         // R.B.G for LPD8806
        colors = strtok(NULL, d);
        char* grn = colors;

        uint8_t redi = atoi(red)/2;
        uint8_t grni = atoi(grn)/2;
        uint8_t blui = atoi(blu)/2;

        flashStripColour = strip.Color(redi, grni, blui);
        tFlashMotionLED.restart();
        Serial.println("FLASH");
        // strip.setPixelColor(0, flashStripColour);
        // strip.setPixelColor(1, flashStripColour);
        // strip.show();
    }
}

/* ----------------------------------------------------------------------------- */

void setup() {

    Serial.begin(9600);
    delay(200);
    Serial.println("Booting");

    strip.begin();
    strip.setPixelColor(0, COLOUR_OFF);
    strip.setPixelColor(1, COLOUR_OFF);
    strip.show();   

    wifiHelper.setupWifi();
    wifiHelper.setupOTA(WIFI_HOSTNAME);
    wifiHelper.setupMqtt();

    wifiHelper.mqttAddSubscription(TOPIC_LIAMROOMTRIP, mqttcallback_DisplayTime);
    wifiHelper.mqttAddSubscription(TOPIC_TIMESTAMP, mqttcallback_Timestamp);
    wifiHelper.mqttAddSubscription(TOPIC_COMMAND, mqttcallback_Command);

    sevenSeg.begin();
    sevenSegClear();
}

void loop() {

    ArduinoOTA.handle();

    wifiHelper.loopMqtt();

    button.serviceEvents();

    runner.execute();

    delay(10);
}

void sevenSegClear()
{
    sevenSeg.setBacklight(LOW_BRIGHT);
    sevenSeg.print("----");
}

void sevenSegPrint(char* disp) {
    sevenSeg.setBacklight(LOW_BRIGHT);
    sevenSeg.print(disp);
}

void sevenSegDisplayTime()
{
    sevenSeg.setBacklight(LOW_BRIGHT);

    int hour12 = hour() > 12 ? hour() - 12 : hour();

    sevenSeg.printTime(hour12, minute(), true); // BLINK?
    // blank leading zero
    if (hour12 < 10)
    {
        sevenSeg.printRaw(sevenSeg.encode(' '), 0);
    }
}

