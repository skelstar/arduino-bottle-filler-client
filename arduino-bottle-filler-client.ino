#include <myWifiHelper.h>
#include <SevenSegmentTM1637.h>
#include <SevenSegmentExtended.h>
#include <TimeLib.h>

char versionText[] = "MQTT Bottle Feeder Client v1.1.0";

#define FEEDBOTTLE_FEED  "/dev/bottleFeed"
#define HHmm_FEED        "/dev/HHmm"
#define MQTT_EVENT_TIMESTAMP    "/dev/timestamp"


#define BOTTLE_FEED_TRIGGER_EV  '1'
#define BOTTLE_FEED_IGNORE_EV  '2'

#define WIFI_OTA_NAME   "arduino-bottle-filler-client"
#define WIFI_HOSTNAME   "arduino-bottle-filler-client"

#define MED_BRIGHT      30
#define LOW_BRIGHT      15
#define BLINK           true

#define CLK 0
#define DIO 2

bool DEBUG = false;

// WIFI ----------------------------------------

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

// Clock ----------------------------------------

SevenSegmentExtended sevenSeg(CLK,DIO);
// http://playground.arduino.cc/Main/SevenSegmentLibrary

// ----------------------------------------------

void mqttcallback_Bottlefeed(byte* payload, unsigned int length) {

    if (payload[0] == BOTTLE_FEED_TRIGGER_EV) {
        sevenSegDisplayTime();
    } else if (payload[0] == BOTTLE_FEED_IGNORE_EV) {
        sevenSeg.print("----");
    }
}

void mqttcallback_Timestamp(byte* payload, unsigned int length) {
    unsigned long pctime = strtoul((char*)payload, NULL, 10);
    setTime(pctime);

    if (hour() == 12) {
        sevenSegClear();
    }
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

    wifiHelper.setupWifi();
    wifiHelper.setupOTA(WIFI_OTA_NAME);

    wifiHelper.setupMqtt();

    wifiHelper.mqttAddSubscription(FEEDBOTTLE_FEED, mqttcallback_Bottlefeed);
    wifiHelper.mqttAddSubscription(MQTT_EVENT_TIMESTAMP, mqttcallback_Timestamp);
}

void loop()
{
    wifiHelper.loopMqtt();

    ArduinoOTA.handle();

    delay(10);
}

//------------------------------------------------------------------

void sevenSegClear() {
    if (hour() > 19 && hour() < 7) {
        sevenSeg.setBacklight(LOW_BRIGHT);
    } else {
        sevenSeg.setBacklight(MED_BRIGHT);
    }
    sevenSeg.print("----");
}

void sevenSegDisplayTime() {
    if (hour() > 19 && hour() < 7) {
        sevenSeg.setBacklight(LOW_BRIGHT);
    } else {
        sevenSeg.setBacklight(MED_BRIGHT);
    }
    sevenSeg.printTime(hour(), minute(), BLINK);
}
