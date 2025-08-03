#include "temperature_sensor.h"
#include <Arduino.h>

// Use sensorID = 1 for this instance
TemperatureSensor::TemperatureSensor(const char* sensorName, const String& sensorID,
                                     Adafruit_ADT7410 temperatureSensor)
    : AbstractSensor("temp", sensorID, sensorName, "CELSIUS"), sensor(temperatureSensor) {}

bool TemperatureSensor::setup() {
    if (!sensor.begin()) {
        return false;
    }
    delay(250);
    sensor.setResolution(ADT7410_16BIT);
    return true;
}

float TemperatureSensor::readData() {
    return sensor.readTempC();
}