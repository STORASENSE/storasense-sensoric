#include "abstract_sensor.h"


AbstractSensor::AbstractSensor(const char* sensorName) : name(sensorName) {}

const char* AbstractSensor::getName() {
    return name;
}

void AbstractSensor::publishData(MqttClient& client, const char* topic) {
    float data = readData();
    client.beginMessage(topic);
    client.print(data);
    client.endMessage();
}
