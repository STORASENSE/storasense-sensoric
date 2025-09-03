#pragma once
#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <vector>
#include "simulated_sensor.h"
#include "sim_presets.h"

extern String makeUUIDv3(const String& nsHex, const String& name);
extern String chipUIDHex();

#ifndef STRESS_NUM_SENSORS
#define STRESS_NUM_SENSORS 5

#endif

#ifndef STRESS_RATE_HZ
#define STRESS_RATE_HZ 1.0f
#endif

#ifndef STRESS_BURST_EVERY_SEC
#define STRESS_BURST_EVERY_SEC 0
#endif

#ifndef STRESS_ADD_DOOR_STATUS
#define STRESS_ADD_DOOR_STATUS 0
#endif

#ifndef STRESS_DOOR_THRESH_CM
#define STRESS_DOOR_THRESH_CM 15.0f
#endif

struct StressCfg {
  const char* sensorType;
  const char* namePrefix;
  const char* unit;
  SimParams   params;
  bool        ultrasonic;
};

class StressSimulator {
public:
  void begin(MqttClient& mqtt, const StressCfg* cfgs, size_t cfgCount) {
    _mqtt = &mqtt;
    _intervalMs = max<uint32_t>(1, (uint32_t)roundf(1000.0f / STRESS_RATE_HZ));
    _nextTick = millis() + 100;

    String ns = chipUIDHex();

    for (size_t i = 0; i < STRESS_NUM_SENSORS; ++i) {
      const StressCfg& c = cfgs[i % cfgCount];

      String name = String(c.namePrefix) + "-" + String(i, 10);

      String uuid = makeUUIDv3(ns, name);

      auto* s = new SimulatedSensor(
        c.sensorType,
        uuid,
        name.c_str(),
        c.unit,
        c.params,
        c.ultrasonic
      );
      s->setup();
      _sns.push_back(s);
    }

    _mqtt->beginWill("storasense/stress/status", false, 0);
    _mqtt->print("offline");
    _mqtt->endWill();
  }

  void loop() {
    const uint32_t now = millis();
    if (now < _nextTick) return;

    if (STRESS_BURST_EVERY_SEC > 0) {
      const uint32_t sec = now / 1000;
      _inBurst = (sec % STRESS_BURST_EVERY_SEC == 0);
    }

    for (size_t i = 0; i < _sns.size(); ++i) {
      _sns[i]->publishData(*_mqtt);

#if STRESS_ADD_DOOR_STATUS
      if (_sns[i]->getSensorType() == String("ULTRASONIC")) {
        float dist = _sns[i]->readData();
        if (dist >= 0.0f) {
          const bool doorOpen = (dist < STRESS_DOOR_THRESH_CM);
          _mqtt->beginMessage("storasense/door/state");
          _mqtt->print(doorOpen ? 1 : 0);
          _mqtt->endMessage();
        }
      }
#endif
    }

    _nextTick = now + (_inBurst ? max<uint32_t>(1, _intervalMs / 5) : _intervalMs);
  }

private:
  MqttClient* _mqtt = nullptr;
  std::vector<SimulatedSensor*> _sns;
  uint32_t _intervalMs = 1000;
  uint32_t _nextTick = 0;
  bool _inBurst = false;
};
