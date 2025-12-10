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
#include "src/MotorController.h"
#include "src/PIDController.h"
#include "src/Navigator.h"
// #include "src/Sonar.h"

// Instantiate objects
NetworkManager network;
LedController led;
LineSensor sensors;
MotorController motors;
PIDController pid(PID_KP, PID_KI, PID_KD);
Navigator navigator;
// Sonar sonar;

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
  
  // Initialize Navigator & Sonar
  // Initialize Navigator
  navigator.begin();
  // sonar.begin();
  
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
  unsigned long currentMillis = millis();

  // Update Network (receive packets)
  network.update();
  
  // Update LED animations
  led.update();
  
  // Check if we received a message
  if (network.hasNewMessage()) {
      String msg = network.getLastMessage();
      Serial.println("Msg: " + msg);
      if (msg.startsWith("CMD:EXPLORE")) {
          navigator.startExploration();
      } else if (msg.startsWith("CMD:STOP")) {
          navigator.stop();
          motors.setSpeeds(0,0);
      }
      led.showPing();
  }
  
  // --- SENSOR READING ---
  uint16_t position = sensors.readLine();
  LineSensor::SensorState sensorState = sensors.getState();
  bool isNode = (sensorState == LineSensor::STATE_NODE);
  bool isLine = (sensorState == LineSensor::STATE_LINE);

  // --- DEBUG OUTPUT ---
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 200) {
      lastPrint = millis();
      Serial.print("SENSORS: [");
      uint16_t* raw = sensors.getRawValues();
      for(int i=0; i<6; i++) {
          Serial.print(raw[i] > 600 ? "X" : "_"); 
      }
      Serial.print("] STATE: ");
      switch(sensorState) {
          case LineSensor::STATE_GAP: Serial.println("GAP"); break;
          case LineSensor::STATE_LINE: Serial.println("LINE"); break;
          case LineSensor::STATE_NODE: Serial.println("NODE"); break;
          case LineSensor::STATE_COMPLEX: Serial.println("COMPLEX"); break;
      }
      // Visual feedback
      if (isNode) led.showPing();
      else led.showLinePosition(position);
  }

  // --- NAVIGATION LOGIC ---
  navigator.update(isNode, isLine, currentMillis);
  NavState state = navigator.getState();

  // --- MOTOR CONTROL ---
  if (state == NAV_IDLE) {
      motors.stop();
      
  } else if (state == NAV_AT_NODE) {
      motors.stop(); 
      
  } else if (state == NAV_TURNING) {
      Direction dir = navigator.getTurnDirection();
      if (dir == DIR_LEFT) {
          motors.setSpeeds(-TURN_SPEED, TURN_SPEED); 
      } else if (dir == DIR_RIGHT) {
          motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      }
      
  } else if (state == NAV_FOLLOWING) {
      // PID Control
      int error = position - 2500; 
      int correction = pid.compute(error);
      
      int leftSpeed = BASE_SPEED - correction;
      int rightSpeed = BASE_SPEED + correction;
      
      leftSpeed = constrain(leftSpeed, -MAX_SPEED, MAX_SPEED);
      rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);
      
      motors.setSpeeds(leftSpeed, rightSpeed);
  }

  delay(1);
}
