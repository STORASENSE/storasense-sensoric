#pragma once
#include <ArduinoMqttClient.h>

// Damit abstract_sensor.cpp die Zeit-Funktion und Startup-Variable kennt
unsigned long getTimestamp();
extern unsigned long startupTs;

class AbstractSensor {
public:
    AbstractSensor(const char* sensorType, const String& sensorID, const char* sensorName, const char* sensorUnit);
    virtual ~AbstractSensor() = default;

    void publishData(MqttClient& client);

    virtual bool   setup()    = 0;
    virtual float  readData() = 0;

protected:
    const char* buildTopic();

private:
    static constexpr const char* BASE_PREFIX   = "dhbw/ai/si2023/4";
    const char*                 sensorType;
    String                      sensorID;
    const char*                 name;
    const char*                 unit;
    static constexpr size_t TOPIC_BUF_SIZE = 16  // strlen("dhbw/ai/si2023/4")
                                        + 1  // slash
                                        + 8  // max. sensorType (z.B. "humidity")
                                        + 1  // slash
                                        + 36 // UUID (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx)
                                        + 1; // null-terminator
    char                        topicBuf[TOPIC_BUF_SIZE];
};