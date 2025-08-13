#include "humidity_sensor.h"
#include <Arduino.h>

bool HumiditySensor::setup() {
    // DHT-Sensor initialisieren
    sensor.begin();
    return true;
}

float HumiditySensor::readData() {
    // Luftfeuchtigkeit auslesen
    float h = sensor.readHumidity();
    // Bei ungültigem Wert 0 zurückgeben
    if (isnan(h)) {
        return 0.0f;
    }
    return h;
}
