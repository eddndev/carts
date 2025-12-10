#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <Arduino.h>
#include "Config.h"

enum NavState {
    NAV_IDLE,
    NAV_FOLLOWING,
    NAV_AT_NODE,
    NAV_TURNING
};

enum TurnState {
    TURN_IDLE,
    TURN_BLIND,   // Phase 1: Blind rotation for X ms
    TURN_CAPTURE  // Phase 2: Sensor polling for line
};

enum Direction {
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
    DIR_NONE
};

struct NavigationNode {
    int id;
    bool visited;
    int edges[4]; // Directions: 0=Up, 1=Right, 2=Down, 3=Left. Store neighbor Node IDs. -1 if no edge.
};

class Navigator {
public:
    Navigator();
    void begin();
    void update(bool nodeDetected, bool lineDetected, unsigned long currentMillis);
    
    void startExploration();
    NavState getState();
    Direction getTurnDirection(); // To tell MotorController which way to spin
    
    // Command Interface
    
    // Command Interface
    void turnLeft();
    void turnRight();
    void goStraight();
    void stop();

private:
    NavState currentState;
    unsigned long lastNodeTime;
    
    // Turn Variables
    TurnState currentTurnState;
    unsigned long turnStartTime;
    Direction targetTurnDirection;
    bool turnComplete;
    
    // DFS / Exploration Variables
    bool isExploring;
    // Simple stack for DFS (fixed size for memory safety on Uno)
    int nodeStack[20];
    int stackPtr;
    
    void handleNodeArrival();
};

#endif
