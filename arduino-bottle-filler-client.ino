#include <myWifiHelper.h>
#include <ArduinoJson.h>            // https://github.com/bblanchon/ArduinoJson
#include <TimeLib.h>


#define WIFI_HOSTNAME "home-bedroom-client"

#define     TOPIC_TIMESTAMP             "/dev/timestamp"
#define     TOPIC_COMMAND               "/device/bedside-client-command"

MyWifiHelper wifiHelper(WIFI_HOSTNAME);

void mqttcallback_Timestamp(byte *payload, unsigned int length) {
    unsigned long pctime = strtoul((char *)payload, NULL, 10);
    setTime(pctime);
    //Serial.print("timestamp: "); Serial.print(hour()); Serial.println(minute());
}

void mqttcallback_Command(byte *payload, unsigned int length) {

    StaticJsonBuffer<50> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(payload);

    if (!root.success()) {
        Serial.println("parseObject() failed");
    }

    const char* command = root["command"];
    const char* value = root["value"];

    root.printTo(Serial);

    Serial.print("command: "); Serial.println(command);
    Serial.print("value: "); Serial.println(value);
}


void setup() {

    Serial.begin(9600);
    delay(200);
    Serial.println("Booting");

    wifiHelper.setupWifi();
    wifiHelper.setupOTA(WIFI_HOSTNAME);
    wifiHelper.setupMqtt();

    wifiHelper.mqttAddSubscription(TOPIC_TIMESTAMP, mqttcallback_Timestamp);
    wifiHelper.mqttAddSubscription(TOPIC_COMMAND, mqttcallback_Command);
}

void loop() {

    ArduinoOTA.handle();

    wifiHelper.loopMqtt();

    delay(10);
}
