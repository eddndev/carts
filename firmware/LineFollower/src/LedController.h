#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include "Arduino_LED_Matrix.h"
#include "Frames.h"

class LedController {
public:
    LedController();
    void begin();
    void update();
    void showPing(); 
    void showSensorValues(uint16_t* values, uint8_t count); 
    void showCalibration(); // Animation during calibration
    void showLinePosition(uint16_t position); // Single dot moving
    void showStop();
    void showExplore();
    void showReset();

private:
    ArduinoLEDMatrix matrix;
    unsigned long lastAnimationTime;
    bool isAnimating;
};

#endif
