#include "humidity_sensor.h"

HumiditySensor::HumiditySensor(const char* sensorName,
                               const String& sensorID,
                               uint8_t pin,
                               uint8_t type)
  : AbstractSensor("HUMIDITY", sensorID, sensorName, "PERCENT"),
    sensor(pin, type)
{}

bool HumiditySensor::setup() {
    sensor.begin();
    return true;
}

float HumiditySensor::readData() {
    float h = sensor.readHumidity();
    if (isnan(h)) {
        return -1.0f;
    }
    return h;
}