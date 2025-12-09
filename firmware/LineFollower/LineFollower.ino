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

// Instantiate objects
ArduinoLEDMatrix matrix;
NetworkManager network;

unsigned long lastPingTime = 0;
const long interval = 2000; // Send ping every 2 seconds

void setup() {
  Serial.begin(115200);
  
  // Matrix initialization
  matrix.begin();
  
  // Network initialization
  network.begin();
}

void loop() {
  // Update Network (receive packets)
  network.update();
  
  // Check if we received a message
  if (network.hasNewMessage()) {
      Serial.println("Action: Packet received!");
      // TODO: Show visual indication on Matrix
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
