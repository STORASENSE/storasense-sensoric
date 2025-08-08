#include "ultrasonic_sensor.h"
#include "abstract_sensor.h"

UltrasonicSensor::UltrasonicSensor(const char* sensorName,
                                   const String& sensorID,
                                   uint8_t trigPin,
                                   uint8_t echoPin)
  : AbstractSensor("ULTRASONIC", sensorID, sensorName, "CM"),
    trig(trigPin), echo(echoPin)
{}

bool UltrasonicSensor::setup() {
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
    return true;
}

float UltrasonicSensor::readData() {
    // Trigger-Puls
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    // Echo messen (Timeout 30000 µs)
    unsigned long duration = pulseIn(echo, HIGH, 30000UL);
    if (duration == 0) {
        // kein Echo
        return -1.0f;
    }

    // Entfernung in cm: (duration·0.0343)/2
    float distance = (duration * 0.0343f) * 0.5f;
    return distance;
}
