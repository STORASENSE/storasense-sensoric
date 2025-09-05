#ifndef SENSORIC_SOLUTION_HUMIDITY_SENSOR_H
#define SENSORIC_SOLUTION_HUMIDITY_SENSOR_H

#include "abstract_sensor.h"
#include <DHT.h>

#define HUMIDITY_SENSOR_PIN  2
#define HUMIDITY_SENSOR_TYPE DHT11

class HumiditySensor : public AbstractSensor {
private:
    DHT sensor;               // das eigentliche DHT-Objekt
public:
    explicit HumiditySensor(const char* sensorName,
                            const String& sensorID,
                            uint8_t pin = HUMIDITY_SENSOR_PIN,
                            uint8_t type = HUMIDITY_SENSOR_TYPE);
    ~HumiditySensor() override = default;

    bool   setup()    override;
    float  readData() override;
};

#endif // SENSORIC_SOLUTION_HUMIDITY_SENSOR_H