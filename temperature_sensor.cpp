// temperature_sensor.cpp
#include "temperature_sensor.h"
#include <Arduino.h>
#include <Wire.h>

TemperatureSensor::TemperatureSensor(const char* sensorName,
                                     Adafruit_ADT7410 temperatureSensor)
  : AbstractSensor(sensorName),
    sensor(temperatureSensor)
{}

bool TemperatureSensor::setup() {
    if (!sensor.begin()) {
        return false;
    }
    delay(250);  // Zeit für den ersten Messwert
    sensor.setResolution(ADT7410_16BIT);
    return true;
}

// Hier die const‑Methoden‑Signatur wie in AbstractSensor
float TemperatureSensor::readData() {
    return sensor.readTempC();
}
