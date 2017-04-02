#include <myWifiHelper.h>
#include <ArduinoJson.h>            // https://github.com/bblanchon/ArduinoJson
#include <Adafruit_NeoPixel.h>
#include <SevenSegmentTM1637.h>
#include <SevenSegmentExtended.h>
#include <TimeLib.h>
#include <string.h>
#include <stdio.h>


#define WIFI_HOSTNAME "home-bedroom-client"

#define     TOPIC_LIAMROOMTRIP          "/node/liamsroom/trip"
#define     TOPIC_TIMESTAMP             "/dev/timestamp"
#define     TOPIC_COMMAND               "/bedroom/bedside-client/command"

#define     DISPLAY_TIME    '1'
#define     CLEAR_TIME      '2'
    
// #defi    ne MED_BRIGHT      30
#define     LOW_BRIGHT  10
#define     CLK         D4       // 0 (-01)
#define     DIO         D5       // 2 (-01)

/* ----------------------------------------------------------------------------- */

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

SevenSegmentExtended sevenSeg(CLK, DIO);
// http://playground.arduino.cc/Main/SevenSegmentLibrary

#define NUM_PIXELS  1
#define PIXEL_PIN   D6  // S3 on NodeMCU
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint32_t    COLOUR_OFF = pixel.Color(0, 0, 0);
uint32_t    currentPixelColor = pixel.Color(0, 1, 0);

/* ----------------------------------------------------------------------------- */

void mqttcallback_Timestamp(byte *payload, unsigned int length) {
    unsigned long pctime = strtoul((char *)payload, NULL, 10);
    setTime(pctime);
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
        char* grn = colors;
        colors = strtok(NULL, d);
        char* blu = colors;

        currentPixelColor = pixel.Color(atoi(red), atoi(grn), atoi(blu));
        pixel.setPixelColor(0, currentPixelColor);
        pixel.show();
    }
}

/* ----------------------------------------------------------------------------- */

void setup() {

    Serial.begin(9600);
    delay(200);
    Serial.println("Booting");

    pixel.begin();
    pixel.setPixelColor(0, currentPixelColor);
    pixel.show();

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

    delay(10);
}

void sevenSegClear()
{
    sevenSeg.setBacklight(LOW_BRIGHT);
    sevenSeg.print("----");
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

