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

// Instantiate objects
NetworkManager network;
LedController led;
LineSensor sensors;

unsigned long lastPingTime = 0;
const long interval = 2000; // Send ping every 2 seconds

void setup() {
  Serial.begin(115200);
  
  // Matrix initialization
  led.begin();
  
  // Initialize Sensors
  sensors.begin();
  
  // Optional: Calibrate on start?
  // let's do a quick manual calibration later, for now we want raw values
  // sensors.calibrate(); 

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
  
  // Read Sensors (Raw for validation)
  uint16_t* rawValues = sensors.getRawValues();
  
  // Visualize Sensors on Matrix
  led.showSensorValues(rawValues, SENSOR_COUNT);
  
  // Debug print periodically
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
      lastPrint = millis();
      Serial.print("Sensors: ");
      for (int i=0; i<SENSOR_COUNT; i++) {
        Serial.print(rawValues[i]);
        Serial.print("\t");
      }
      Serial.println();
  }

  // Send "Ping" periodically
  unsigned long currentMillis = millis();
  if (currentMillis - lastPingTime >= interval) {
      lastPingTime = currentMillis;
      String pingMsg = "PING from " + String(WiFi.localIP()[3]); // Send last octet of IP
      network.sendPacket(pingMsg);
      Serial.println("Sent: " + pingMsg);
  }
  
  delay(10); // Small delay for stability
}
