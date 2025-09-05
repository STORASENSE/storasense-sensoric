#ifndef SENSORIC_SOLUTION_TEMPERATURE_SENSOR_H
#define SENSORIC_SOLUTION_TEMPERATURE_SENSOR_H

#include "abstract_sensor.h"
#include <Adafruit_ADT7410.h>

// ADT7410 temperature sensor class
class TemperatureSensorInside  : public AbstractSensor {
private:
    Adafruit_ADT7410 sensor;
public:
    // Constructor: sensorName (MQTT label), configured Adafruit_ADT7410 instance
    explicit TemperatureSensorInside (const char* sensorName, const String& sensorID, Adafruit_ADT7410 temperatureSensorInside);
    ~TemperatureSensorInside () override = default;

    bool setup() override;
    float readData() override;
};

#endif