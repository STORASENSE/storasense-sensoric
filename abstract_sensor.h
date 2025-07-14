#include <ArduinoMqttClient.h>

#ifndef SENSORIC_SOLUTION_ABSTRACT_SENSOR_H
#define SENSORIC_SOLUTION_ABSTRACT_SENSOR_H


/**
 * This class represents a sensor on an Arduino board. The method <code>AbstractSensor::readData</code>
 * can be used to read data from the sensor, and needs to be overridden in appropriate subclasses.
 *
 * Additionally, this class provides a functionality to send the sensor's data to an MQTT broker.
 * This can be achieved with the <code>AbstractSensor::publishData</code> method, and is implemented
 * by default.
 */
class AbstractSensor {

private:
    /**
     * The sensor's name.
     */
    char[] name;

public:

    /**
     * Creates a sensor.
     * @param sensorName The sensor's name.
     */
    AbstractSensor(char[] sensorName);

    ~AbstractSensor() = default;

    /**
     * Retrieves the sensor's name.
     * @return The sensor's name.
     */
    char[] getName();

    /**
     * Publishes the read data from this sensor to the MQTT server.
     * Specifically, invokes <code>AbstractSensor::readData</code> to read data from this sensor
     * and then sends the data as a message to the given MQTT topic
     * using the given MQTT client.
     * @param client The MQTT client
     * @param topic The MQTT topic to publish the data to
     */
    void publishData(MqttClient client, char[] topic);

    /**
     * This method should be executed once to setup the sensor.
     * @return Whether the setup was successful
     */
    virtual bool setup() = 0;

    /**
     * Reads data from the sensor and returns it as a float.
     * @return The sensor's read data
     */
    virtual float readData() = 0;

};

#endif