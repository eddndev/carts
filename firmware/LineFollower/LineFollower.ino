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
  // Wait 3 seconds before starting motors so user can place robot
  Serial.println("Get Ready! Starting in 3 seconds...");
  led.showReset(); // Show "Filling" animation as readiness
  delay(3000);
  
  // Network initialization
  Serial.println("Connecting to WiFi...");
  led.showExplore(); // Show "Radar" while finding WiFi
  network.begin(); // BLOCKING until connected
  
  led.showStop(); // Ready state (Motors 0)
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Update Network
  network.update();
  
  // 2. Update LED animations
  led.update();
  
  // 3. Handle Commands
  if (network.hasNewMessage()) {
      String msg = network.getLastMessage();
      Serial.println("Msg: " + msg);
      if (msg.startsWith("CMD:EXPLORE")) {
          navigator.startExploration();
          led.showExplore();
      } else if (msg.startsWith("CMD:STOP")) {
          navigator.stop();
          motors.setSpeeds(0,0);
          led.showStop();
      } else if (msg.startsWith("CMD:RESET")) {
          navigator.stop();
          motors.setSpeeds(0,0);
          led.showReset();
          delay(1000); 
          led.showStop();
      }
  }
  
  // 4. Sensor Reading
  uint16_t position = sensors.readLine();
  LineSensor::SensorState sensorState = sensors.getState();
  bool isNode = (sensorState == LineSensor::STATE_NODE);
  bool isLine = (sensorState == LineSensor::STATE_LINE);

  // 5. Update Navigation Logic
  navigator.update(isNode, isLine, currentMillis);
  NavState state = navigator.getState();

  // 6. Debug Output & Visual Feedback
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
      
      // Update LED Matrix based on high-level state
      if (state == NAV_FOLLOWING) {
           led.showLinePosition(position);
      } else if (isNode) {
           led.showPing();
      }
      // Note: STOP/EXPLORE/RESET animations are triggered by commands and persist 
      // until overridden or timeout, unless showLinePosition interrupts them.
  }

  // 7. Motor Control
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
