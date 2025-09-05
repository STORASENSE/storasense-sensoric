#include "ultrasonic_sensor.h"
#include "abstract_sensor.h"

UltrasonicSensor::UltrasonicSensor(const char* sensorName,
                                   const String& sensorID,
                                   uint8_t trigPin,
                                   uint8_t echoPin)
  : AbstractSensor("ULTRASONIC", sensorID, sensorName, "CENTIMETER"),
    trig(trigPin), echo(echoPin)
{}

bool UltrasonicSensor::setup() {
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
    return true;
}

float UltrasonicSensor::readData() {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    unsigned long duration = pulseIn(echo, HIGH, 30000UL);
    if (duration == 0) {
        return -1.0f;
    }

    float distance = (duration * 0.0343f) * 0.5f;
    return distance;
}
