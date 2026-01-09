/*
  LineFollower.ino
  Main firmware for Arduino Uno R4 WiFi Line Follower Cart

  Features:
  - WiFi P2P Communication
  - LED Matrix Status Display
  - Line Following Logic
*/

#include "Arduino_LED_Matrix.h"
#include "WiFiS3.h"
#include "src/LedController.h"
#include "src/LineSensor.h"
#include "src/MotorController.h"
#include "src/Navigator.h"
#include "src/NetworkManager.h"
#include "src/PIDController.h"
#include <Arduino.h>

// Instantiate objects
NetworkManager network;
LedController led;
LineSensor sensors;
MotorController motors;
PIDController pid(PID_KP, PID_KI, PID_KD);
Navigator navigator;

unsigned long lastPingTime = 0;
const long interval = 2000; // Send ping every 2 seconds

// Telemetry Timer
unsigned long lastTelemetryTime = 0;

void setup() {
  Serial.begin(115200);

  // Matrix initialization
  led.begin();

  // Initialize Motors
  motors.begin();

  // Initialize Sensors
  sensors.begin();

  // Initialize Navigator
  navigator.begin();

  // Calibration Sequence
  Serial.println("Starting Calibration...");
  led.showCalibration();
  sensors.calibrate();
  Serial.println("Calibration Complete.");

  // Wait 3 seconds before starting motors so user can place robot
  Serial.println("Get Ready! Starting in 3 seconds...");
  led.showReset(); // Show "Filling" animation as readiness
  delay(3000);

  // Network initialization (Non-Blocking)
#if ENABLE_WIFI
  Serial.println("Initializing Network Manager...");
  network.begin(); // Will start connecting in background
#else
  Serial.println("OFFLINE MODE: Skipping Network.");
  Serial.println("Auto-starting Autonomous Mode...");
  navigator.startAutonomous(); // Start Simple Following immediately
  led.showExplore(); // Show "Searching"/Moving animation
#endif
}

void loop() {
  unsigned long currentMillis = millis();

  // 0. Sensor Reading (FIRST THING: Get fresh, calibrated data)
  uint16_t position = sensors.readLine();
  LineSensor::SensorState sensorState = sensors.getState();
  bool isNode = (sensorState == LineSensor::STATE_NODE);
  bool isLine = (sensorState == LineSensor::STATE_LINE);

#if ENABLE_WIFI
  // 1. Update Network (State Machine)
  network.update();
  
  // 2. Check Connection State
  static bool wasConnected = false; 
  bool isConnected = network.isConnected();

  if (!isConnected) {
      wasConnected = false; 
      motors.stop(); // SAFETY STOP
      
      if (network.isConnecting()) {
          led.showExplore(); 
      } else {
          led.showStop(); 
      }
      
      led.update();
      return; // SKIP the rest of the loop until connected
  }
  
  if (!wasConnected && isConnected) {
      wasConnected = true;
      led.showStop(); 
      Serial.println("Reconnected! LED set to Ready.");
  }
#endif

  // 3. Update LED animations
  led.update();

#if ENABLE_WIFI
  // Periodic Heartbeat (2s)
  if (currentMillis - lastPingTime > interval) {
      lastPingTime = currentMillis;
      if (network.isConnected()) {
          network.sendPacket("PONG:CartFollower");
      }
  }

  // --- SENSOR TELEMETRY (200ms) ---
  if (currentMillis - lastTelemetryTime >= 200) {
    lastTelemetryTime = currentMillis;

    // Format: {"s":<state>, "v":[s0,s1,s2,s3,s4,s5]}
    String json = "{";
    json += "\"s\":" + String(navigator.getState()); 
    uint16_t* sensorValues = sensors.getRawValues();
    json += ",\"v\":[";
    for(int i=0; i<SENSOR_COUNT; i++) {
       // Normalize or just send raw? Sending raw (0-1000) is safer, app handles threshold.
       json += String(sensorValues[i]);
       if(i < SENSOR_COUNT-1) json += ",";
    }
    json += "]}";
    
    // Broadcast telemetry
    network.broadcast(json);
  }
#endif

#if ENABLE_WIFI
  // 4. Handle Commands
  if (network.hasNewMessage()) {
    led.showPacketReceived(); // Visual Flash
    String msg = network.getLastMessage();
    Serial.println("Msg: " + msg);

    // Delegar comandos de navegación al Navigator
    if (msg.startsWith("NAV:")) {
      String navCmd = msg.substring(4);
      navigator.processExternalCommand(navCmd);
      network.respondToLastSender("ACK:" + navCmd);
    }
    // Comandos de sistema
    else if (msg.startsWith("CMD:AUTO")) {
      navigator.startAutonomous();
      led.showExplore();
    } else if (msg.startsWith("CMD:STOP")) {
      navigator.stop();
      motors.setSpeeds(0, 0);
      led.showStop();
      network.respondToLastSender("ACK:STOP");
    } else if (msg.startsWith("CMD:RESET")) {
      navigator.stop();
      motors.setSpeeds(0, 0);
      led.showReset();
      delay(1000);
      led.showStop();
      delay(1000);
      led.showStop();
    } else if (msg.startsWith("CMD:PING")) {
      network.respondToLastSender("ACK:PING");
      led.showPacketReceived();
    } else if (msg.startsWith("CMD:CALIBRATE")) {
      led.showCalibration();
      sensors.calibrate();
      network.respondToLastSender("ACK:CALIBRATE");
      led.showStop();
    }
    // TEST Commands
    else if (msg.startsWith("TEST:FWD")) {
      motors.setSpeeds(BASE_SPEED, BASE_SPEED);
      network.respondToLastSender("ACK:FWD");
    } else if (msg.startsWith("TEST:BWD")) {
      motors.setSpeeds(-BASE_SPEED, -BASE_SPEED);
      network.respondToLastSender("ACK:BWD");
    } else if (msg.startsWith("TEST:LEFT")) {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      network.respondToLastSender("ACK:LEFT");
    } else if (msg.startsWith("TEST:RIGHT")) {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      network.respondToLastSender("ACK:RIGHT");
    }
  }
#endif

  // 4. Sensor Reading (Moved to top of loop)

  // 5. Update Navigation Logic
  navigator.update(isNode, isLine, currentMillis);
  NavState state = navigator.getState();

  // 6. Debug Output
  static unsigned long lastPrint = 0;
  static NavState lastReportedState = NAV_IDLE;

  if (millis() - lastPrint > 500) { // Slowed down UART debug to prioritize UDP
    lastPrint = millis();
    Serial.print("SENSORS: [");
    uint16_t *raw = sensors.getRawValues();
    for (int i = 0; i < 6; i++) {
      Serial.print(raw[i] > 600 ? "X" : "_");
    }
    Serial.print("] STATE: ");
    Serial.println(state);

    // Broadcast state changes (Event driven)
    if (state != lastReportedState) {
      lastReportedState = state;
      // Also broadcast status change to network
#if ENABLE_WIFI
      char buffer[32];
      sprintf(buffer, "STATUS_CHANGE:%d", state);
      network.broadcast(buffer);
#endif
    }
    
    // Matrix updates
    if (state == NAV_FOLLOWING) {
      led.showLinePosition(position);
    } else if (isNode) {
      led.showPing();
    }
  }

  // 7. Motor Control
  if (state == NAV_IDLE || state == NAV_WAITING_HOST) {
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
