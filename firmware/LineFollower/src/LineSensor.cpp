#include "LineSensor.h"

LineSensor::LineSensor() {
}

void LineSensor::begin() {
    qtr.setTypeAnalog(); // QTR-8A
    qtr.setSensorPins(SENSOR_PINS, SENSOR_COUNT);
    
    // Optional: set emitter pin if used, otherwise they are always on or tied to VCC
    // qtr.setEmitterPin(2); 
}

void LineSensor::calibrate() {
    Serial.println("Calibrating sensors... Move sensor over line!");
    
    // Calibrate for approx 5 seconds (200 * 25ms?) no, just loops
    // usually 400 iterations is good
    for (uint16_t i = 0; i < 400; i++) {
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
