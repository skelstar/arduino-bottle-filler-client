#include <myWifiHelper.h>
#include <SevenSegmentTM1637.h>
#include <SevenSegmentExtended.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>            // https://github.com/bblanchon/ArduinoJson
#include <TaskScheduler.h>

char versionText[] = "MQTT Bottle Feeder Client v2.0";

#define     TOPIC_FEEDBOTTLE            "/dev/bottleFeed"
#define     TOPIC_LIAMROOMTRIP          "/node/liam-room-trip"
#define     TOPIC_TIMESTAMP             "/dev/timestamp"
#define     TOPIC_COMMAND               "/device/bedside-client-command"

#define DISPLAY_TIME    '1'
#define CLEAR_TIME      '2'

#define WIFI_OTA_NAME "arduino-bottle-filler-client-nodemcu"
#define WIFI_HOSTNAME "arduino-bottle-filler-client-nodemcu"

// #define MED_BRIGHT      30
#define LOW_BRIGHT 10
#define BLINK true

#define CLK         7       // 0 (-01)
#define DIO         8       // 2 (-01)
#define PIXEL_PIN   0

#define NUM_PIXELS  1

bool DEBUG = false;

// WIFI ----------------------------------------

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

// Clock ----------------------------------------

SevenSegmentExtended sevenSeg(CLK, DIO);
// http://playground.arduino.cc/Main/SevenSegmentLibrary

// Pixel -----------------------------------------

//***Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

//***uint32_t COLOUR_OFF = pixel.Color(0, 0, 0);

//***uint32_t currentPixelColor = pixel.Color(0, 0, 0);

/*---------------------------------------------------------------------*/

Scheduler runner;

// void tCallback_FlashTriggerLEDON();
// void tCallback_FlashTriggerLEDOFF();

// Task tFlashTriggerLED(500, TASK_FOREVER, &tCallback_FlashTriggerLEDON, &runner, false);

// void tCallback_FlashTriggerLEDON() {
//     pixel.begin();
//     pixel.setPixelColor(0, currentPixelColor);
//     pixel.show();
//     tFlashTriggerLED.setCallback(tCallback_FlashTriggerLEDOFF);
// }

// void tCallback_FlashTriggerLEDOFF() {
//     pixel.begin();
//     pixel.setPixelColor(0, COLOUR_OFF);
//     pixel.show();
//     tFlashTriggerLED.setCallback(tCallback_FlashTriggerLEDON);
// }

// ----------------------------------------------

void mqttcallback_DisplayTime(byte *payload, unsigned int length) {

    if (payload[0] == DISPLAY_TIME) {
        sevenSegDisplayTime();
    }
    else if (payload[0] == CLEAR_TIME) {
        sevenSeg.print("----");
    }
    //***pixel.begin();
    //***pixel.setPixelColor(0, COLOUR_OFF);
   //***pixel.show();

}

void mqttcallback_Timestamp(byte *payload, unsigned int length) {
    unsigned long pctime = strtoul((char *)payload, NULL, 10);
    setTime(pctime);

    //***pixel.begin();
    //***pixel.setPixelColor(0, COLOUR_OFF);
    //***pixel.show();

}

void mqttcallback_Command(byte *payload, unsigned int length) {

    StaticJsonBuffer<200> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject((char*)payload);

    if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
    }

    const char* command = root["command"];
    const char* value = root["value"];

    Serial.print("command: "); Serial.println(command);
    Serial.print("value: "); Serial.println(value);

    if (strcmp(command, "Test") == 0) {
        Serial.println("Command = Test");
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

    //***pixel.begin();
    //***pixel.setPixelColor(0, COLOUR_OFF);
    //***pixel.show();

    wifiHelper.setupWifi();
    wifiHelper.setupOTA(WIFI_OTA_NAME);

    wifiHelper.setupMqtt();

    wifiHelper.mqttAddSubscription(TOPIC_LIAMROOMTRIP, mqttcallback_DisplayTime);
    wifiHelper.mqttAddSubscription(TOPIC_TIMESTAMP, mqttcallback_Timestamp);
    wifiHelper.mqttAddSubscription(TOPIC_COMMAND, mqttcallback_Command);

    //runner.addTask(tFlashTriggerLED);
}

void loop()
{
    wifiHelper.loopMqttNonBlocking();

    ArduinoOTA.handle();

    runner.execute();

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
