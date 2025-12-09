#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

class MotorController {
public:
    MotorController();
    void begin();
    
    // speed: -255 to 255 (Negative for reverse)
    void setSpeeds(int leftSpeed, int rightSpeed);
    
    // Convenience methods
    void stop();
    void forward(int speed);
    void backward(int speed);
    void turnLeft(int speed);
    void turnRight(int speed);

private:
    void setMotor(int pinPWM, int pinIN1, int pinIN2, int speed);
};

#endif
