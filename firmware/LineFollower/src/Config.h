#ifndef CONFIG_H
#define CONFIG_H

// --- Network Configuration ---

// Set to true to make this device the Access Point (Host network)
// Set to false to connect to an existing network (Client)
#define IS_ACCESS_POINT true 

// WiFi Credentials
// If IS_ACCESS_POINT is true: This device CREATES this network.
// If IS_ACCESS_POINT is false: This device CONNECTS to this network.
#define SECRET_SSID "ArduinoCartNet"
#define SECRET_PASS "cartpassword123"

// Communication
#define UDP_PORT 4210
#define BROADCAST_IP "255.255.255.255" // Broadcast to all local devices

// --- Sensors & Actuators ---
// QTR-8A (Analog) Sensor Pins
// We use 6 sensors connected to Analog pins (A0-A5)
const uint8_t SENSOR_PINS[] = {A0, A1, A2, A3, A4, A5};
const uint8_t SENSOR_COUNT = 6;

// --- Motors (L298N) ---
// Left Motor (Physically connected to M2 pins)
#define PIN_M1_EN 11 
#define PIN_M1_IN1 13 // Swapped 12/13 to fix "Backward" issue
#define PIN_M1_IN2 12

// Right Motor (Physically connected to M1 pins)
#define PIN_M2_EN 3  
#define PIN_M2_IN3 2 // Assuming standard (check next test)
#define PIN_M2_IN4 4

// --- PID & Speed Control ---
#define PID_KP 0.1   // Proportional (Start small)
#define PID_KI 0.0   // Integral (Usually 0 for line followers)
#define PID_KD 0.0   // Derivative (Crucial for damping)

#define BASE_SPEED 130 // Must be > MIN_PWM !
#define MAX_SPEED 200  
#define TURN_SPEED 150 

// Deadband Correction (Measured)
#define MIN_PWM_L 105
#define MIN_PWM_R 97

#endif
