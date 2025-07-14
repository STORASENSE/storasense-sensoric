#include "abstract_sensor.h"


AbstractSensor::AbstractSensor(char[] sensorName) : name(sensorName) {}

char[] AbstractSensor::getName() {
    return name;
}

void AbstractSensor::publishData(MqttClient client, char[] topic) {
    float data = readData();
    client.beginMessage(topic);
    client.print(data);
    client.endMessage();
}
