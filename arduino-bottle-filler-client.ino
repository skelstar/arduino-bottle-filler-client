#include <myWifiHelper.h>
#include <SevenSegmentTM1637.h>
#include <SevenSegmentExtended.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>            // https://github.com/bblanchon/ArduinoJson

char versionText[] = "MQTT Bottle Feeder Client v2.0";

#define     TOPIC_FEEDBOTTLE            "/dev/bottleFeed"
#define     TOPIC_TIMESTAMP             "/dev/timestamp"
#define     TOPIC_TEMP_LIAM_ROOM_LEVEL  "/dev/temperature_liam_room_level"

#define BOTTLE_FEED_TRIGGER_EV '1'
#define BOTTLE_FEED_IGNORE_EV '2'

#define WIFI_OTA_NAME "arduino-bottle-filler-client"
#define WIFI_HOSTNAME "arduino-bottle-filler-client"

// #define MED_BRIGHT      30
#define LOW_BRIGHT 10
#define BLINK true

#define CLK         0
#define DIO         2
#define PIXEL_PIN   0

#define NUM_PIXELS  1

bool DEBUG = false;

// WIFI ----------------------------------------

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

// Clock ----------------------------------------

SevenSegmentExtended sevenSeg(CLK, DIO);
// http://playground.arduino.cc/Main/SevenSegmentLibrary

// Pixel -----------------------------------------

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint32_t COLOUR_OFF = pixel.Color(0, 0, 0);

// ----------------------------------------------

void mqttcallback_Bottlefeed(byte *payload, unsigned int length) {

    if (payload[0] == BOTTLE_FEED_TRIGGER_EV)
    {
        sevenSegDisplayTime();

        pixel.begin();
        pixel.setPixelColor(0, COLOUR_OFF);
        pixel.show();
    }
    else if (payload[0] == BOTTLE_FEED_IGNORE_EV)
    {
        sevenSeg.print("----");
    }
}

void mqttcallback_Timestamp(byte *payload, unsigned int length) {
    unsigned long pctime = strtoul((char *)payload, NULL, 10);
    setTime(pctime);
    // reset the display at lunchtime
    if (hour() == 12) {
        sevenSegClear();
    }
}

void mqttcallback_TempLevel(byte *payload, unsigned int length) {

    StaticJsonBuffer<200> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject((char*)payload);

    if (!root.success()) {
        Serial.println("parseObject() failed");
        // pixel.begin();
        // pixel.setPixelColor(0, pixel.Color(255,0,0));
        // pixel.show();
        return;
    }

    bool flash = root["flash"];
    int level = root["level"];
    int colourRed = root["colour"][0];
    int colourGrn = root["colour"][1];
    int colourBlu = root["colour"][2];

    Serial.print("flash: "); Serial.println(flash);
    Serial.print("level: "); Serial.println(level);

    pixel.begin();
    pixel.setPixelColor(0, pixel.Color(colourRed, colourGrn, colourBlu));
    pixel.show();
}

// ----------------------------------------------

void setup()
{
    Serial.begin(9600);
    delay(50);
    Serial.println("Booting");
    Serial.println(versionText);

    sevenSeg.begin();
    sevenSegClear();

    pixel.begin();
    pixel.setPixelColor(0, COLOUR_OFF);
    pixel.show();

    wifiHelper.setupWifi();
    wifiHelper.setupOTA(WIFI_OTA_NAME);

    wifiHelper.setupMqtt();

    wifiHelper.mqttAddSubscription(TOPIC_FEEDBOTTLE, mqttcallback_Bottlefeed);
    wifiHelper.mqttAddSubscription(TOPIC_TIMESTAMP, mqttcallback_Timestamp);
    wifiHelper.mqttAddSubscription(TOPIC_TEMP_LIAM_ROOM_LEVEL, mqttcallback_TempLevel);
    wifiHelper.mqttAddSubscription(TOPIC_TEMP_LIAM_ROOM_LEVEL, mqttcallback_TempLevel);
}

void loop()
{
    wifiHelper.loopMqttNonBlocking();

    ArduinoOTA.handle();

    delay(10);
}

//------------------------------------------------------------------

void sevenSegClear()
{
    sevenSeg.setBacklight(LOW_BRIGHT);
    sevenSeg.print("----");
}

void sevenSegDisplayTime()
{
    sevenSeg.setBacklight(LOW_BRIGHT);

    int hour12 = hour() > 12 ? hour() - 12 : hour();

    sevenSeg.printTime(hour12, minute(), BLINK);
    // blank leading zero
    if (hour12 < 10)
    {
        sevenSeg.printRaw(sevenSeg.encode(' '), 0);
    }
}
