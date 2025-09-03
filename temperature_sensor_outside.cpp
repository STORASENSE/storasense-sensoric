#include "temperature_sensor_outside.h"

TemperatureSensorOutside::TemperatureSensorOutside(const char* sensorName,
                                                   const String& sensorID,
                                                   uint8_t dataPin)
: AbstractSensor("TEMPERATURE_OUTSIDE", sensorID, sensorName, "CELSIUS"),
  _pin(dataPin),
  _oneWire(dataPin),
  _sensors(&_oneWire)
{}

bool TemperatureSensorOutside::setup() {
  _sensors.begin();
  _sensors.setResolution(12);
  _sensors.setWaitForConversion(true);
  return true;
}

bool TemperatureSensorOutside::setAddress(const DeviceAddress addr) {
  for (int i = 0; i < 8; ++i) _addr[i] = addr[i];
  _useAddress = true;
  return true;
}

float TemperatureSensorOutside::readData() {
    if (_useAddress) _sensors.requestTemperaturesByAddress(_addr);
    else             _sensors.requestTemperatures();

    float tC = _useAddress ? _sensors.getTempC(_addr)
                          : _sensors.getTempCByIndex(_index);

    if (tC == DEVICE_DISCONNECTED_C) {
      Serial.println("[DS18B20] DEVICE_DISCONNECTED");
      return -1.0f;
    }
    if (tC < -60.0f || tC > 150.0f) {
      Serial.print("[DS18B20] Implausible: "); Serial.println(tC);
      return -1.0f;
    }
    return tC;
}