#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Network Configuration ---

// Set to true to make this device the Access Point (Host network)
// Set to false to connect to an existing network (Client)
#define IS_ACCESS_POINT false

// WiFi Credentials
// If IS_ACCESS_POINT is true: This device CREATES this network.
// If IS_ACCESS_POINT is false: This device CONNECTS to this network.
#define SECRET_SSID "CartControl"
#define SECRET_PASS "0102030405"

// Communication
#define UDP_PORT 4210
#define BROADCAST_IP "255.255.255.255" // Broadcast to all local devices

// --- Sensors & Actuators ---
// QTR-8A (Analog) Sensor Pins
// We use 6 sensors connected to Analog pins (A0-A5)
const uint8_t SENSOR_PINS[] = {A5, A4, A3, A2, A1, A0};
const uint8_t SENSOR_COUNT = 6;
#define PIN_SENSOR_EMITTER                                                     \
  6 // Connect 'LEDON' or 'EMITTER' pin here for ambient light rejection

// --- Motors (L298N) ---
// Left Motor (D2, D3, D4)
#define PIN_M1_EN 3
#define PIN_M1_IN1 2
#define PIN_M1_IN2 4

// Right Motor (D11, D12, D13)
#define PIN_M2_EN 11
#define PIN_M2_IN3 12
#define PIN_M2_IN4 13

// --- PID & Speed Control ---
#define PID_KP 0.09 // Increased for sharper turns (Mid-point)
#define PID_KI 0.0
#define PID_KD 1.0 // High Damping maintained

#define BASE_SPEED 90 // Compromise speed for control + torque
#define MAX_SPEED 200 // Allow faster corrections
#define TURN_SPEED 150

// Voltage Safety Limit for 11.1V Battery -> 6V Motors
// Calculation: (6V / 11.1V) * 255 ~= 138. Safe limit: 120 (~5.2V).
#define MAX_PWM_LIMIT 92

// Deadband Correction (Measured)
// Lowered slightly to rely on kinetic friction
#define MIN_PWM_L 90
#define MIN_PWM_R 85

// Speed Matching Factors (0.0 to 1.0)
#define SPEED_FACTOR_L 1.0
#define SPEED_FACTOR_R 0.95

// --- SONAR (HC-SR04) ---
// LOGIC DISABLED IN MAIN LOOP, DEFINES KEPT FOR COMPILATION
#define PIN_SONAR_TRIG 12
#define PIN_SONAR_ECHO 11
#define SONAR_MAX_DIST_CM 200
#define OBSTACLE_DIST_CM 15

// --- NODE DETECTION ---
#define NODE_THICKNESS_MS 100 // Time all sensors must be black to count as Node
#define NODE_COOLDOWN_MS 1000 // Debounce time after leaving a node

#endif
