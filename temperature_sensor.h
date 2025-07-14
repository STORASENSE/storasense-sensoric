#include "abstract_sensor.h"
#include "Adafruit_ADT7410.h"

#ifndef SENSORIC_SOLUTION_TEMPERATURE_SENSOR_H
#define SENSORIC_SOLUTION_TEMPERATURE_SENSOR_H


/**
 * This class represents a temperature sensor, specifically the sensor <code>Adafruit_ADT7410</code>.
 * The sensor needs to be installed to the Arduino before it can be used!
 */
class TemperatureSensor : public AbstractSensor {

private:
    Adafruit_ADT7410 sensor;

public:
    /**
     * Instantiates a temperature sensor.
     * @param temperatureSensor The sensor's name
     * @param temperatureSensor The implemented temperature sensor.
     */
    TemperatureSensor(char[] sensorName, Adafruit_ADT7410 temperatureSensor);

    ~TemperatureSensor();

    bool setup() override;

    /**
     * Reads the temperature from the configured sensor and returns it in °C.
     * @return The temperature read from the configured sensor, in degrees Celsius
     */
    float readData() override;

};

#endif
