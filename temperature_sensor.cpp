#include "temperature_sensor.h"

#include "Arduino.h"
#include <Wire.h>


TemperatureSensor::TemperatureSensor(char[] sensorName, Adafruit_ADT7410 temperatureSensor) : AbstractSensor(sensorName), sensor(temperatureSensor) {}

TemperatureSensor::~TemperatureSensor() {
    delete sensor;
}

bool TemperatureSensor::setup() {
    if (!sensor.begin()) {
        return false;
    }
    delay(250); // sensor takes 250 ms to get first readings
    sensor.setResolution(ADT7410_16BIT); // set sensor resolution to 16 bits
    return true;
}

float TemperatureSensor::readData() {
    return sensor.readTempC();
}
