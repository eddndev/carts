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
// Left Motor
#define PIN_M1_EN 3  // PWM
#define PIN_M1_IN1 2
#define PIN_M1_IN2 4

// Right Motor
#define PIN_M2_EN 11 // PWM (Check R4 Pinout, 11 is usually PWM)
#define PIN_M2_IN3 12
#define PIN_M2_IN4 13

#endif
