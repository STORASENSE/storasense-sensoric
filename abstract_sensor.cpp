#include "abstract_sensor.h"
#include <ArduinoJson.h>
#include <cstdio>  // für snprintf

// JSON-Kapazität: timestamp, value[], sequence, meta.startup + Puffer
static const size_t JSON_CAPACITY =
  JSON_OBJECT_SIZE(4) +
  JSON_ARRAY_SIZE(1) +
  JSON_OBJECT_SIZE(1) +
  128;

// 1) Konstruktor implementieren
AbstractSensor::AbstractSensor(const char* sensorType_,
                               const String& sensorID_,
                               const char* sensorName_,
                               const char* sensorUnit_)
  : sensorType(sensorType_),
    sensorID(sensorID_),
    name(sensorName_),
    unit(sensorUnit_)
{}

// 2) buildTopic implementieren
const char* AbstractSensor::buildTopic() {
    snprintf(topicBuf,
            TOPIC_BUF_SIZE,
            "%s/%s/%s",
            BASE_PREFIX,
            sensorType,
            sensorID.c_str());
    return topicBuf;
}

// 3) publishData wie gewünscht
void AbstractSensor::publishData(MqttClient& client) {
    float value = readData();

    StaticJsonDocument<JSON_CAPACITY> doc;
    doc["timestamp"] = getTimestamp();

    JsonArray arr = doc.createNestedArray("value");
    arr.add(value);


    static unsigned long sequence = 0;
    doc["sequence"] = sequence++;

    JsonObject meta = doc.createNestedObject("meta");
    meta["startup"] = startupTs;
    meta["unit"] = unit;

    const char* topic = buildTopic();
    client.beginMessage(topic);
    serializeJson(doc, client);
    client.endMessage();
}