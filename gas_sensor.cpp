#include "gas_sensor.h"
#include "abstract_sensor.h"

static constexpr float VCC  = 5.0f;
static constexpr float RL   = 10e3f;
static constexpr float R0   = 9.8e3f;
static constexpr float A    = 773.387f;
static constexpr float B    = -2.518f;

GasSensor::GasSensor(const char* sensorName, const String& sensorID, uint8_t sensorPin)
  : AbstractSensor("CO2", sensorID, sensorName, "PPM"), pin(sensorPin)
{}

bool GasSensor::setup() {
    pinMode(pin, INPUT);
    return true;
}

float GasSensor::readData() {
    int raw = analogRead(pin);

    float vout = raw / 1023.0f * VCC;

    float rs = RL * (VCC / vout - 1.0f);

    float ratio = rs / R0;

    float ppm = A * powf(ratio, B);

    return ppm;
}