#include <myWifiHelper.h>
#include <ArduinoJson.h>            // https://github.com/bblanchon/ArduinoJson

#define WIFI_HOSTNAME "home-bedroom-client"

#define     TOPIC_TIMESTAMP             "/dev/timestamp"
#define     TOPIC_COMMAND               "/device/bedside-client-command"

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

void mqttcallback_Timestamp(byte *payload, unsigned int length) {
    // unsigned long pctime = strtoul((char *)payload, NULL, 10);
    // setTime(pctime);
    Serial.println("timestamp");
}


void setup() {

    Serial.begin(9600);
    delay(200);
    Serial.println("Booting");

    wifiHelper.setupWifi();
    wifiHelper.setupOTA(WIFI_HOSTNAME);
    wifiHelper.setupMqtt();

    wifiHelper.mqttAddSubscription(TOPIC_TIMESTAMP, mqttcallback_Timestamp);
}

void loop() {

    ArduinoOTA.handle();

    wifiHelper.loopMqtt();

    delay(10);
}
