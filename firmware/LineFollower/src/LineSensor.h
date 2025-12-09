#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <Arduino.h>
#include <QTRSensors.h>
#include "Config.h"

class LineSensor {
public:
    LineSensor();
    void begin();
    void calibrate(); // Blocking calibration routine
    uint16_t readLine(); // Returns position (0 to 5000 for 6 sensors)
    uint16_t* getRawValues(); // For debugging

private:
    QTRSensors qtr;
    uint16_t trustedSensorValues[SENSOR_COUNT];
};

#endif
