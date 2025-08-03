#ifndef SENSORIC_SOLUTION_HUMIDITY_SENSOR_H
#define SENSORIC_SOLUTION_HUMIDITY_SENSOR_H

#include "abstract_sensor.h"
#include <DHT.h>

// Default pin and sensor type for the humidity sensor
#define HUMIDITY_SENSOR_PIN  2
#define HUMIDITY_SENSOR_TYPE DHT11  // oder DHT22

// DHT-basierter Luftfeuchte-Sensor
class HumiditySensor : public AbstractSensor {
private:
    DHT sensor;
public:
    // Konstruktor: MQTT-Label (default: "humidity")
    explicit HumiditySensor(const char* sensorName = "humidity", const String& sensorID = String())
      : AbstractSensor("hum", sensorID, sensorName, "PERCENT"),
        sensor(HUMIDITY_SENSOR_PIN, HUMIDITY_SENSOR_TYPE)
    {}

    ~HumiditySensor() override = default;

    bool setup() override;
    float readData() override;
};

#endif // SENSORIC_SOLUTION_HUMIDITY_SENSOR_H
