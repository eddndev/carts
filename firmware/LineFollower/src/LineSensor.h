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
    
    // Debug / State Logic
    enum SensorState {
        STATE_GAP,      // No line (all white)
        STATE_LINE,     // Normal line
        STATE_NODE,     // Thick intersection (all black)
        STATE_COMPLEX   // Multiple detached segments (Bifurcation?)
    };
    
    SensorState getState();
    bool isNodeDetected(); // Keep for legacy compatibility if needed

private:
    QTRSensors qtr;
    uint16_t trustedSensorValues[SENSOR_COUNT];
};

#endif
