#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include "Config.h"
#include <Arduino.h>

enum NavState {
  NAV_IDLE,
  NAV_FOLLOWING,
  NAV_AT_NODE,
  NAV_TURNING,
  NAV_WAITING_HOST // Hybrid Arch: Pause for Host
};

enum TurnState {
  TURN_IDLE,
  TURN_BLIND,  // Phase 1: Blind rotation for X ms
  TURN_CAPTURE // Phase 2: Sensor polling for line
};

enum Direction { DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_NONE };

class Navigator {
public:
  Navigator();
  void begin();
  void update(bool nodeDetected, bool lineDetected,
              unsigned long currentMillis);

  void startAutonomous(); // Renamed from startExploration
  void stop();

  NavState getState();
  Direction getTurnDirection(); 

  // Command Interface
  void processExternalCommand(String cmd); 

  void turnLeft();
  void turnRight();
  void goStraight();

private:
  NavState currentState;
  unsigned long lastNodeTime;

  // Turn Variables
  TurnState currentTurnState;
  unsigned long turnStartTime;
  Direction targetTurnDirection;

  bool isAutonomous;

  void handleNodeArrival();
};

#endif
