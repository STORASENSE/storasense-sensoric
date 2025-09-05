#include "temperature_sensor_inside.h"
#include "abstract_sensor.h"
#include <Arduino.h>

// Use sensorID = 1 for this instance
TemperatureSensorInside ::TemperatureSensorInside (const char* sensorName, const String& sensorID,
                                     Adafruit_ADT7410 temperatureSensorInside)
    : AbstractSensor("TEMPERATURE_INSIDE", sensorID, sensorName, "CELSIUS"), sensor(temperatureSensorInside) {}

bool TemperatureSensorInside::setup() {
    if (!sensor.begin()) {
        return false;
    }
    delay(250);
    sensor.setResolution(ADT7410_16BIT);
    return true;
}

float TemperatureSensorInside::readData() {
    return sensor.readTempC();
}