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
#include "src/NetworkManager.h"
#include "src/LedController.h"
#include "src/LineSensor.h"
#include "src/MotorController.h"
#include "src/PIDController.h"

// Instantiate objects
NetworkManager network;
LedController led;
LineSensor sensors;
MotorController motors;
PIDController pid(PID_KP, PID_KI, PID_KD);

unsigned long lastPingTime = 0;
const long interval = 2000; // Send ping every 2 seconds

void setup() {
  Serial.begin(115200);
  
  // Matrix initialization
  led.begin();
  
  // Initialize Motors
  motors.begin();

  // Initialize Sensors
  sensors.begin();
  
  // Calibration Sequence
  Serial.println("Starting Calibration...");
  led.showCalibration();
  sensors.calibrate(); 
  Serial.println("Calibration Complete.");
  
  // Wait 3 seconds before starting motors so user can place robot
  Serial.println("Get Ready! Starting in 3 seconds...");
  delay(3000);
  
  // Network initialization
  network.begin();
}

void loop() {
  // Update Network (receive packets)
  network.update();
  
  // Update LED animations
  led.update();
  
  // Check if we received a message
  if (network.hasNewMessage()) {
      Serial.println("Action: Packet received!");
      led.showPing();
  }
  
  // Read Line Position
  uint16_t position = sensors.readLine();
  
  // Visualize Position on Matrix
  led.showLinePosition(position);
  
  // --- PID CONTROL ---
  int error = position - 2500; // 0-5000 -> -2500 to 2500
  int correction = pid.compute(error);
  
  // Standard steering: Line Right -> Pos 5000 -> Err + -> Left Speed Increase -> Turn Right
  int leftSpeed = BASE_SPEED + correction;
  int rightSpeed = BASE_SPEED - correction;
  
  // Constrain speeds
  leftSpeed = constrain(leftSpeed, -MAX_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);
  
  // Apply to motors
  // Apply to motors
  motors.setSpeeds(leftSpeed, rightSpeed); // ENABLE MOVEMENT

  // Debug print periodically
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 200) { // Fast frequency (200ms) for tuning
      lastPrint = millis();
      Serial.print("Pos: ");
      Serial.print(position);
      Serial.print(" Err: ");
      Serial.print(error);
      Serial.print(" Corr: ");
      Serial.print(correction);
      Serial.print(" L: ");
      Serial.print(leftSpeed);
      Serial.print(" R: ");
      Serial.println(rightSpeed);
  }

  // Send "Ping" periodically
  unsigned long currentMillis = millis();
  if (currentMillis - lastPingTime >= interval) {
      lastPingTime = currentMillis;
      String pingMsg = "PING from " + String(WiFi.localIP()[3]); // Send last octet of IP
      network.sendPacket(pingMsg);
  }
  
  delay(1); // Minimize delay for loop speed
}
