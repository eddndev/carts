/*
  LineFollower.ino
  Main firmware for Arduino Uno R4 WiFi Line Follower Cart
  
  Features:
  - WiFi P2P Communication
  - LED Matrix Status Display
  - Line Following Logic
*/

#include <Arduino.h>
#include "WiFiS3.h"
#include "Arduino_LED_Matrix.h"

// Instantiate objects
ArduinoLEDMatrix matrix;

void setup() {
  Serial.begin(115200);
  
  // Initialize Matrix
  matrix.begin();
  
  // TODO: Add WiFi setup
  // TODO: Add Motor setup
}

void loop() {
  // Main control loop
  delay(100);
}
