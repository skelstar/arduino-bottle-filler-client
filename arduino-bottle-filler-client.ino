#include <myWifiHelper.h>
#include <SevenSegmentTM1637.h>
#include <SevenSegmentExtended.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>

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

#define LEVEL_OFF   '0'
#define LEVEL_COLD  '1'
#define LEVEL_COOL  '2'
#define LEVEL_OK    '3'
#define LEVEL_WARM  '4'
#define LEVEL_HOT   '5'

uint32_t COLOUR_OFF = pixel.Color(0, 0, 0);
uint32_t COLOUR_COLD = pixel.Color(0, 0, 100);
uint32_t COLOUR_COOL = pixel.Color(0, 0, 5);
uint32_t COLOUR_OK = pixel.Color(0, 5, 0);
uint32_t COLOUR_WARM = pixel.Color(5, 0, 0);
uint32_t COLOUR_HOT = pixel.Color(100, 0, 0);

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

    pixel.begin();

    switch (payload[0]) {
        case LEVEL_COLD:
            pixel.setPixelColor(0, COLOUR_COLD);
            break;
        case LEVEL_COOL:
            pixel.setPixelColor(0, COLOUR_COOL);
            break;
        case LEVEL_OK:
            pixel.setPixelColor(0, COLOUR_OK);
            break;
        case LEVEL_WARM:
            pixel.setPixelColor(0, COLOUR_WARM);
            break;
        case LEVEL_HOT:
            pixel.setPixelColor(0, COLOUR_HOT);
            break;
        default:
            pixel.setPixelColor(0, COLOUR_OFF);
    }
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
}

void loop()
{
    wifiHelper.loopMqtt();

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
