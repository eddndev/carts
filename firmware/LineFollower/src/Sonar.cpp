#include "Sonar.h"

Sonar::Sonar() {
    currentDistance = SONAR_MAX_DIST_CM;
    lastPingTime = 0;
}

void Sonar::begin() {
    pinMode(PIN_SONAR_TRIG, OUTPUT);
    pinMode(PIN_SONAR_ECHO, INPUT);
}

void Sonar::update() {
    // Non-blocking update every 100ms
    if (millis() - lastPingTime < 100) return;
    lastPingTime = millis();

    // Trigger Pulse
    digitalWrite(PIN_SONAR_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_SONAR_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_SONAR_TRIG, LOW);

    // Read Echo (timeout 18ms ~ 3m)
    long duration = pulseIn(PIN_SONAR_ECHO, HIGH, 18000); 

    if (duration == 0) {
        currentDistance = SONAR_MAX_DIST_CM; // Out of range
    } else {
        currentDistance = duration * 0.034 / 2;
    }
}

float Sonar::getDistanceCm() {
    return currentDistance;
}

bool Sonar::isObstacleDetected() {
    return (currentDistance > 0 && currentDistance < OBSTACLE_DIST_CM);
}
