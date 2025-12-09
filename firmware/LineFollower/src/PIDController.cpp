#include "PIDController.h"

PIDController::PIDController(float kp, float ki, float kd) {
    this->Kp = kp;
    this->Ki = ki;
    this->Kd = kd;
    
    this->target = 2500; // Default center for QTR-8 (0-5000)
    this->lastError = 0;
    this->integral = 0;
}

void PIDController::setTunings(float kp, float ki, float kd) {
    this->Kp = kp;
    this->Ki = ki;
    this->Kd = kd;
}

void PIDController::setTarget(int target) {
    this->target = target;
}

int PIDController::compute(int error) {
    // P term
    float P = error;
    
    // I term
    integral += error;
    float I = integral;
    
    // D term
    float D = error - lastError;
    lastError = error;
    
    // PID Calculation
    float output = (Kp * P) + (Ki * I) + (Kd * D);
    
    return (int)output;
}
