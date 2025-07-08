#ifndef SENSOR_BASE_H
#define SENSOR_BASE_H

class SensorBase {
public:
    virtual void setup() = 0;
    virtual void readData() = 0;
    virtual void publishData() = 0;
};

#endif