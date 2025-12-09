#include "MotorController.h"

MotorController::MotorController() {
}

void MotorController::begin() {
    // Left Motor
    pinMode(PIN_M1_EN, OUTPUT);
    pinMode(PIN_M1_IN1, OUTPUT);
    pinMode(PIN_M1_IN2, OUTPUT);
    
    // Right Motor
    pinMode(PIN_M2_EN, OUTPUT);
    pinMode(PIN_M2_IN3, OUTPUT);
    pinMode(PIN_M2_IN4, OUTPUT);
    
    stop();
}

void MotorController::stop() {
    setSpeeds(0, 0);
}

void MotorController::forward(int speed) {
    setSpeeds(speed, speed);
}

void MotorController::backward(int speed) {
    setSpeeds(-speed, -speed);
}

void MotorController::turnLeft(int speed) {
    setSpeeds(-speed, speed);
}

void MotorController::turnRight(int speed) {
    setSpeeds(speed, -speed);
}

void MotorController::setSpeeds(int leftSpeed, int rightSpeed) {
    // Apply Speed Matching Factors
    leftSpeed = leftSpeed * SPEED_FACTOR_L;
    rightSpeed = rightSpeed * SPEED_FACTOR_R;

    // Apply Deadband Correction for Left Motor
    if (leftSpeed > 0) {
        leftSpeed = map(leftSpeed, 0, 255, MIN_PWM_L, 255);
    } else if (leftSpeed < 0) {
        leftSpeed = map(leftSpeed, -255, 0, -255, -MIN_PWM_L);
    }

    // Apply Deadband Correction for Right Motor
    if (rightSpeed > 0) {
        rightSpeed = map(rightSpeed, 0, 255, MIN_PWM_R, 255);
    } else if (rightSpeed < 0) {
        rightSpeed = map(rightSpeed, -255, 0, -255, -MIN_PWM_R);
    }

    setMotor(PIN_M1_EN, PIN_M1_IN1, PIN_M1_IN2, leftSpeed);
    setMotor(PIN_M2_EN, PIN_M2_IN3, PIN_M2_IN4, rightSpeed);
}

void MotorController::setMotor(int pinPWM, int pinIN1, int pinIN2, int speed) {
    // constrain speed to -255 to 255
    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;
    
    if (speed > 0) {
        // Forward
        digitalWrite(pinIN1, HIGH);
        digitalWrite(pinIN2, LOW);
        analogWrite(pinPWM, speed);
    } else if (speed < 0) {
        // Backward
        digitalWrite(pinIN1, LOW);
        digitalWrite(pinIN2, HIGH);
        analogWrite(pinPWM, -speed); // PWM is always positive
    } else {
        // Stop (Coast or Brake? Let's use Brake: HIGH/HIGH or LOW/LOW)
        // LOW/LOW is "Coast" (wheels spin free)
        // HIGH/LOW + 0 PWM is Brake
        // Let's use simple LOW/LOW for now to save power
        digitalWrite(pinIN1, LOW);
        digitalWrite(pinIN2, LOW);
        analogWrite(pinPWM, 0);
    }
}
