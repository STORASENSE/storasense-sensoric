#pragma once
#include "abstract_sensor.h"
#include <Arduino.h>

class GasSensor : public AbstractSensor {
public:
    // sensorName: MQTT-Label, sensorID: eindeutige ID, sensorPin: analoger Pin (Standard A0)
    explicit GasSensor(const char* sensorName, const String& sensorID, uint8_t sensorPin = A0);
    ~GasSensor() override = default;

    bool  setup() override;
    float readData() override;

private:
    uint8_t pin;
};