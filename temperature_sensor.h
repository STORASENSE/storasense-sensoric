#include "abstract_sensor.h"
#include "Adafruit_ADT7410.h"

// temperature_sensor.h
#ifndef SENSORIC_SOLUTION_TEMPERATURE_SENSOR_H
#define SENSORIC_SOLUTION_TEMPERATURE_SENSOR_H

/**
 * Diese Klasse repräsentiert einen ADT7410-Temperatursensor.
 */
class TemperatureSensor : public AbstractSensor {
private:
    Adafruit_ADT7410 sensor;  // Wertobjekt, kein Pointer!

public:
    /**
     * Konstruktor
     * @param sensorName Name für AbstractSensor
     * @param temperatureSensor Fertig konfiguriertes Adafruit_ADT7410‑Objekt
     */
    explicit TemperatureSensor(const char* sensorName,
                               Adafruit_ADT7410 temperatureSensor);

    ~TemperatureSensor() override = default;

    bool setup() override;
    float readData() override;
};

#endif // SENSORIC_SOLUTION_TEMPERATURE_SENSOR_H
