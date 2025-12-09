#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>

class PIDController {
public:
    PIDController(float kp, float ki, float kd);
    void setTunings(float kp, float ki, float kd);
    int compute(int error);
    void setTarget(int target); // Usually 2500 (Center)

private:
    float Kp;
    float Ki;
    float Kd;
    
    int target;
    int lastError;
    long integral;
};

#endif
