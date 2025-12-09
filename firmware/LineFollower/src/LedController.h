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
    void showPing(); // Call this when packet received
    void showSensorValues(uint16_t* values, uint8_t count); // Visualization

private:
    ArduinoLEDMatrix matrix;
    unsigned long lastAnimationTime;
    bool isAnimating;
};

#endif
