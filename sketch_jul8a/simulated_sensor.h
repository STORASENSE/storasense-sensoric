#pragma once
#include "abstract_sensor.h"
#include "sim_generator.h"

class SimulatedSensor : public AbstractSensor {
public:
  SimulatedSensor(const char* sensorType,
                  const String& sensorID,
                  const char* sensorName,
                  const char* sensorUnit,
                  const SimParams& params,
                  bool ultrasonic = false)
  : AbstractSensor(sensorType, sensorID, sensorName, sensorUnit),
    gen((uint32_t)micros(), params),
    isUltrasonic(ultrasonic) {}

  bool setup() override { return true; }

  float readData() override {
    gen.tick();
    float v = gen.value();
    if (isnan(v)) return -1.0f;
    return v;
  }

  bool startupFlag() const { return gen.startup(); }

private:
  SimGenerator gen;
  bool isUltrasonic;
};