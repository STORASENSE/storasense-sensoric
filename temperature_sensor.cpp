#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "Arduino.h"
#include "Adafruit_ADT7410.h"
#include <Wire.h>
#include "sensor_base.h"

class TemperatureSensor : public SensorBase {
  private:
    Adafruit_ADT7410 tempsensor = Adafruit_ADT7410();
    float temperature;
    //Client* mqttClient;

  public:
    TemperatureSensor() {}

    void setup() override {
      Serial.begin(115200);
      Serial.println("ADT7410 demo");
      // Make sure the sensor is found, you can also pass in a different i2c
      // address with tempsensor.begin(0x49) for example
      if (!tempsensor.begin()) {
        Serial.println("Couldn't find ADT7410!");
        while (1);
      }
      // sensor takes 250 ms to get first readings
      delay(250);
      // ** Optional **
      // Can set ADC resolution
      // ADT7410_13BIT = 13 bits (default)
      // ADT7410_16BIT = 16 bits
      tempsensor.setResolution(ADT7410_16BIT);
      Serial.print("Resolution = ");
      switch (tempsensor.getResolution()) {
        case ADT7410_13BIT:
          Serial.print("13 ");
          break;
        case ADT7410_16BIT:
          Serial.print("16 ");
          break;
        default:
          Serial.print("??");
      }
      Serial.println("bits");
    }

    void readData() override {
        temperature = tempsensor.readTempC();

        Serial.print(temperature); 
        delay(1000);
    }

    void publishData() override {
    /*    if (!isnan(temperature)) {
            char payload[10];
            snprintf(payload, sizeof(payload), "%.2f", temperature);
            mqttClient->publish("sensor/temperature", payload);
        } */ 
    }
};

#endif