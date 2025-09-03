#pragma once
#include "abstract_sensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#ifndef DS18B20_DATA_PIN
#define DS18B20_DATA_PIN 8
#endif

class TemperatureSensorOutside : public AbstractSensor {
public:
  explicit TemperatureSensorOutside(const char* sensorName,
                                    const String& sensorID,
                                    uint8_t dataPin = DS18B20_DATA_PIN);

  ~TemperatureSensorOutside() override = default;

  bool  setup() override;
  float readData() override;

  void setIndex(uint8_t index) { _index = index; _useAddress = false; }
  bool setAddress(const DeviceAddress addr);

private:
  uint8_t           _pin;
  OneWire           _oneWire;
  DallasTemperature _sensors;

  uint8_t       _index = 0;
  bool          _useAddress = false;
  DeviceAddress _addr{};
};