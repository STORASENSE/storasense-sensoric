#include "gas_sensor.h"
#include "abstract_sensor.h"

// Betriebsspannung
static constexpr float VCC  = 5.0f;
// Lastwiderstand RL (z.B. 10 kΩ)
static constexpr float RL   = 10e3f;
// R0 im sauberen (kalibrierten) Luftzustand (z.B. 9.8 kΩ)
static constexpr float R0   = 9.8e3f;
// Parameter der Empfindlichkeitsgeraden (aus Datenblatt, LPG-Kurve)
static constexpr float A    = 773.387f;
static constexpr float B    = -2.518f;

GasSensor::GasSensor(const char* sensorName, const String& sensorID, uint8_t sensorPin)
  : AbstractSensor("CO2", sensorID, sensorName, "PPM"), pin(sensorPin)
{}

bool GasSensor::setup() {
    pinMode(pin, INPUT);
    // Hinweis: MQ-2 braucht eine Vorheizzeit von ca. 3 Minuten,
    // damit sich der Heizdraht auf Temperatur bringt und stabile Werte liefert.
    // Ihr könnt hier delay(180000) einfügen oder extern vor Start der Messungen vorheizen.
    return true;
}

float GasSensor::readData() {
    // 1) Roh-ADC-Wert holen
    int raw = analogRead(pin);

    // 2) Spannung am Sensor
    float vout = raw / 1023.0f * VCC;

    // 3) Sensor-Widerstand Rs
    float rs = RL * (VCC / vout - 1.0f);

    // 4) Verhältnis zu R0
    float ratio = rs / R0;

    // 5) PPM aus Empfindlichkeitsgerade: y = A * x^B
    float ppm = A * powf(ratio, B);

    return ppm;
}