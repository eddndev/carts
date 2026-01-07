#include "Navigator.h"

Navigator::Navigator() {
  currentState = NAV_IDLE;
  isAutonomous = false;
  lastNodeTime = 0;

  currentTurnState = TURN_IDLE;
  turnStartTime = 0;
  targetTurnDirection = DIR_NONE;
}

void Navigator::begin() { currentState = NAV_IDLE; }

void Navigator::update(bool nodeDetected, bool lineDetected,
                       unsigned long currentMillis) {
  if (currentState == NAV_IDLE)
    return;

  // Debounce Node Detection
  if (nodeDetected && (currentMillis - lastNodeTime > NODE_COOLDOWN_MS)) {
    // Only trigger node if we are FOLLOWING (not already turning or stuck)
    if (currentState == NAV_FOLLOWING) {
      lastNodeTime = currentMillis;
      
#if ENABLE_WIFI
      // Hybrid Architecture: Stop and wait for Host (App) instruction
      // Even in "AUTO" mode, we wait for the App to send "GO_STRAIGHT" to keep logic consistent
      // unless we want 'Smart Autonomous' on firmware. 
      // User said: "App will receive Node Reached and respond GO_STRAIGHT".
      currentState = NAV_WAITING_HOST;
      Serial.println("NAV: Node Reached. Waiting for Host...");
#else
      // Offline Mode: Self-manage
      handleNodeArrival();
#endif
    }
  }

  // --- TURN LOGIC UPDATE ---
  if (currentState == NAV_TURNING) {
    if (currentTurnState == TURN_BLIND) {
      // Phase 1: Blind Wait
      if (currentMillis - turnStartTime >
          300) { // 300ms blind spin (adjust for 90 deg approx)
        currentTurnState = TURN_CAPTURE;
      }
    } else if (currentTurnState == TURN_CAPTURE) {
      // Phase 2: Wait for Center Sensor (Line Capture)
      if (lineDetected) {
        Serial.println("NAV: Turn Complete (Line Captured)");
        currentState = NAV_FOLLOWING;
        currentTurnState = TURN_IDLE;
      }

      // Timeout safety (1.5s max)
      if (currentMillis - turnStartTime > 1500) {
        Serial.println("NAV: Turn Timeout!");
        currentState = NAV_FOLLOWING; // Try to recover
        currentTurnState = TURN_IDLE;
      }
    }
  }
}

void Navigator::startAutonomous() {
  isAutonomous = true;
  currentState = NAV_FOLLOWING;
  Serial.println("NAV: Starting Autonomous Mode (Simple)");
}

NavState Navigator::getState() { return currentState; }

void Navigator::handleNodeArrival() {
  currentState = NAV_AT_NODE;
  Serial.println("NAV: Node Detected!");

  if (isAutonomous) {
    // Simple Logic: Priority Straight -> Left
    Serial.println("NAV: Auto Decision: Straight/Forward");
    goStraight(); 
  }
}

Direction Navigator::getTurnDirection() { return targetTurnDirection; }

void Navigator::turnLeft() {
  currentState = NAV_TURNING;
  currentTurnState = TURN_BLIND;
  targetTurnDirection = DIR_LEFT;
  turnStartTime = millis();
}

void Navigator::turnRight() {
  currentState = NAV_TURNING;
  currentTurnState = TURN_BLIND;
  targetTurnDirection = DIR_RIGHT;
  turnStartTime = millis();
}

void Navigator::goStraight() { currentState = NAV_FOLLOWING; }

void Navigator::stop() { currentState = NAV_IDLE; isAutonomous = false; }

void Navigator::processExternalCommand(String cmd) {
  if (cmd == "GO_LEFT")
    turnLeft();
  else if (cmd == "GO_RIGHT")
    turnRight();
  else if (cmd == "GO_STRAIGHT")
    goStraight();
  else if (cmd == "WAIT")
    currentState = NAV_WAITING_HOST;
  // else if (cmd == "STOP") stop(); // Optional

  Serial.print("NAV: Executed Command: ");
  Serial.println(cmd);
}
