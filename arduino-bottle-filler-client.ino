#include <myWifiHelper.h>
#include <SevenSegmentTM1637.h>
#include <SevenSegmentExtended.h>

char versionText[] = "MQTT Bottle Feeder Client v1.0.0";

#define FEEDBOTTLE_FEED  "/dev/bottleFeed"
#define HHmm_FEED        "/dev/HHmm"

#define BOTTLE_FEED_EV  '1'
#define DELETE_FEED_EV  '2'

#define WIFI_OTA_NAME   "arduino-bottle-filler-client"
#define WIFI_HOSTNAME   "arduino-bottle-filler-client"

#define MED_BRIGHT      30
#define LOW_BRIGHT      15

#define CLK 0
#define DIO 2

// WIFI ----------------------------------------

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

// Clock ----------------------------------------

SevenSegmentExtended sevenSeg(CLK,DIO);

volatile int hour = 0;
volatile int minute = 0;

volatile bool callback_event;

// ----------------------------------------------

void bottlefeed_callback(byte* payload, unsigned int length) {

    if (payload[0] == BOTTLE_FEED_EV) {
        sevenSegDisplayTime();
    } else if (payload[0] == DELETE_FEED_EV) {
        clearTimeDisp();
        sevenSeg.print("----");
    }
}

void devtime_callback(byte* payload, unsigned int length) {

    if (payload[0] == '-') {
        hour = -1;
    } else {
        hour = (payload[0]-'0') * 10;
        hour += payload[1]-'0';
        minute = (payload[3]-'0') * 10;
        minute += payload[4]-'0';
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

    wifiHelper.mqttAddSubscription(FEEDBOTTLE_FEED, bottlefeed_callback);
    wifiHelper.mqttAddSubscription(HHmm_FEED, devtime_callback);
}

void loop()
{
    wifiHelper.loopMqtt();

    ArduinoOTA.handle();

    delay(10);
}

//------------------------------------------------------------------

void clearTimeDisp() {
}

void sevenSegClear() {
    sevenSeg.setBacklight(MED_BRIGHT);
    sevenSeg.print("----");
    clearTimeDisp();
}

void sevenSegDisplayTime() {
    sevenSeg.setBacklight(MED_BRIGHT);
    sevenSeg.printTime(hour, minute, true);
}
