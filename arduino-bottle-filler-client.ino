#include <myWifiHelper.h>
#include <SevenSegmentTM1637.h>
#include <SevenSegmentExtended.h>

char versionText[] = "MQTT Bottle Feeder Client v0.9.0";

// WIFI ----------------------------------------

#define FEEDBOTTLE_FEED  "/dev/bottleFeed"

#define WIFI_OTA_NAME   "arduino-bottle-filler-client"
#define WIFI_HOSTNAME   "arduino-bottle-filler-client"

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

// Clock ----------------------------------------

#define MED_BRIGHT      30
#define LOW_BRIGHT      15

char TimeDisp[] = "--:--";

#define CLK 0
#define DIO 2

SevenSegmentExtended sevenSeg(CLK,DIO);

volatile int hour = 0;
volatile int minute = 0;

volatile bool callback_event;

// ----------------------------------------------

void bottlefeed_callback(byte* payload, unsigned int length) {

	if (payload[0] == '-') {
		hour = -1;
	} else {
	    hour = (payload[0]-'0') * 10;
	    hour += payload[1]-'0';
	    minute = (payload[3]-'0') * 10;
	    minute += payload[4]-'0';
	}
	callback_event = true;
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
}

void loop()
{
    wifiHelper.loopMqtt();

    ArduinoOTA.handle();

    if (callback_event) {
    	if (hour >= 0) {
    		sevenSegDisplayTime();
    	} else {
			clearTimeDisp();
	    	sevenSeg.print("----");
    	}
    	Serial.println("callback!");
    	callback_event = false;
    }

    delay(10);
}

//------------------------------------------------------------------

void clearTimeDisp() {
    TimeDisp[0] = '-';
    TimeDisp[1] = '-';
    TimeDisp[2] = ':';
    TimeDisp[3] = '-';
    TimeDisp[4] = '-';
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
