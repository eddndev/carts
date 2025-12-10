#ifndef SONAR_H
#define SONAR_H

#include <Arduino.h>
#include "Config.h"

class Sonar {
public:
    Sonar();
    void begin();
    void update(); // Call frequently
    float getDistanceCm();
    bool isObstacleDetected();

private:
    unsigned long lastPingTime;
    float currentDistance;
};

#endif
