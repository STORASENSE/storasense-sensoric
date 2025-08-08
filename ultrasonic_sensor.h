#pragma once
#include "abstract_sensor.h"
#include <Arduino.h>

/**
 * @brief Klasse für den HC-SR04 Ultraschall-Sensor
 *
 * Misst die Entfernung in cm und veröffentlicht sie als Float-Wert.
 * Zusätzlich kann so ein Tür-Öffnen-Zustand erkannt werden, indem ein Schwellwert gesetzt wird.
 */
class UltrasonicSensor : public AbstractSensor {
public:
    /**
     * @param sensorName MQTT-Label
     * @param sensorID   eindeutige UUID
     * @param trigPin    Pin für Trigger-Signal
     * @param echoPin    Pin für Echo-Signal
     */
    UltrasonicSensor(const char* sensorName, const String& sensorID,
                     uint8_t trigPin, uint8_t echoPin);
    ~UltrasonicSensor() override = default;

    bool  setup() override;
    float readData() override;

private:
    uint8_t trig;
    uint8_t echo;
};