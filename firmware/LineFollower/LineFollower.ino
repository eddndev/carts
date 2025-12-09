/*
  LineFollower.ino - Speed Matching Test
  Runs straight for 2 seconds to check drift.
*/

#include <Arduino.h>
#include "src/MotorController.h"

MotorController motors;

// Tuning variable: Reduce faster motor
// If curves LEFT -> Right is faster (or Left slower) -> Reduce Right? No, Left is dragging.
// Ideally, reduce the FASTER motor.
float leftFactor = 1.0;
float rightFactor = 0.95; // Reduced to correct left drift 

// Initial Speed
int speed = 100; // 0-255 (Before deadband map)

void setup() {
  Serial.begin(115200);
  motors.begin();
  
  Serial.println("--- SPEED MATCHING TEST ---");
  Serial.println("Robot will drive STRAIGHT for 2s.");
  Serial.println("Watch the curve!");
  delay(2000);
}

void loop() {
  Serial.println("GO!");
  // Apply factors manually here to test, then put in Config
  // Current guess: 1.0 vs 1.0
  motors.setSpeeds(speed * leftFactor, speed * rightFactor);
  
  delay(2000);
  
  motors.stop();
  Serial.println("STOP. Waiting 3s...");
  delay(3000);
}
