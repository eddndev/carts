#include "LineSensor.h"

LineSensor::LineSensor() {
}

void LineSensor::begin() {
    qtr.setTypeAnalog(); // QTR-8A
    qtr.setSensorPins(SENSOR_PINS, SENSOR_COUNT);
    
    // Optional: set emitter pin if used, otherwise they are always on or tied to VCC
    qtr.setEmitterPin(PIN_SENSOR_EMITTER); 
}

void LineSensor::calibrate() {
    Serial.println("Calibrating sensors... Move sensor over line!");
    
    // Calibrate for approx 3 seconds (150 iters)
    for (uint16_t i = 0; i < 150; i++) {
        qtr.calibrate();
        
        // Print progress every 20 iterations
        if (i % 20 == 0) Serial.print(".");
    }
    Serial.println("\nCalibration Done.");
}

uint16_t LineSensor::readLine() {
    return qtr.readLineBlack(trustedSensorValues);
}

uint16_t* LineSensor::getRawValues() {
    // Just read raw without position calculation for debugging
    qtr.read(trustedSensorValues);
    return trustedSensorValues;
}

bool LineSensor::isNodeDetected() {
    return (getState() == STATE_NODE);
}

LineSensor::SensorState LineSensor::getState() {
    // 1. Count black sensors (threshold > 600)
    int blackCount = 0;
    bool sensorIsBlack[SENSOR_COUNT];
    
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        sensorIsBlack[i] = (trustedSensorValues[i] > 600);
        if (sensorIsBlack[i]) blackCount++;
    }

    // 2. Identify State
    if (blackCount == 0) return STATE_GAP;
    if (blackCount >= 5) return STATE_NODE; // 5 or 6 sensors black -> Node

    // 3. Check for Bifurcations (Segments)
    // A segment is a continuous block of black sensors.
    // "Black-Black-White-Black" -> 2 segments
    int segments = 0;
    bool inSegment = false;
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        if (sensorIsBlack[i]) {
            if (!inSegment) {
                segments++;
                inSegment = true;
            }
        } else {
            inSegment = false;
        }
    }

    if (segments > 1) return STATE_COMPLEX; // Disconnected lines found

    return STATE_LINE; // Default single segment
}
