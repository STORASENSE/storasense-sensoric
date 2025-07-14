# Sensoric_Solution

# Sensors

The class `AbstractSensor` serves as a basis for all sensors and
provides functionalities for reading sensor data and for publishing
sensor data to an MQTT broker under a given topic.
Currently, the following sensor implementations exist:

- A temperature sensor, which is provided by the `TemperatureSensor`
  class. This is a wrapper around `Adafruit_ADT7410`. Note that only
  this specific type of sensor can be detected.

## Adding new sensors

In order to add implementations for new sensors, extend the class
`AbstractSensor` and override the `setup` and `readData` functions.
