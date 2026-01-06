#include "NetworkManager.h"

NetworkManager::NetworkManager() {
  newMessageAvailable = false;
  lastPingTime = 0;
  state = DISCONNECTED;
  connectionAttempts = 0;
  lastConnectionAttempt = 0;
}

void NetworkManager::begin() {
  // Check for WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication failed: WiFi module not found!");
    state = OFFLINE;
    return;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

#if IS_ACCESS_POINT
  // AP Mode (Can be blocking for now, or minimal delay)
  WiFi.beginAP(SECRET_SSID, SECRET_PASS); 
  // AP is usually instant on Local modules
  state = CONNECTED; // Assume success for AP for simplicity in hybrid model
  
#else
  Serial.println("[WiFi] Starting Connection Process (Non-Blocking)...");
  state = DISCONNECTED; // Will trigger connect in first update
#endif

  // UDP Begin immediately to be ready
  Udp.begin(UDP_PORT);
}

void NetworkManager::update() {
  unsigned long currentMillis = millis();

#if !IS_ACCESS_POINT
  // State Machine for Connection
  switch (state) {
    case DISCONNECTED:
      // blocking nature of WiFi.begin() on R4 causes "freezing".
      // We increase the delay significantly so the user sees animation 
      // for at least a few seconds between frozen attempts.
      if (currentMillis - lastConnectionAttempt > 5000) { // Wait 5s before retry
         Serial.print("[WiFi] Attempting connection to: ");
         Serial.println(SECRET_SSID);
         WiFi.begin(SECRET_SSID, SECRET_PASS); // This is blocking for several seconds on fail
         state = CONNECTING;
         lastConnectionAttempt = currentMillis;
         connectionAttempts++;
      }
      break;
      
    case CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
          IPAddress ip = WiFi.localIP();
          // Wait for valid IP (DHCP can take a moment after WL_CONNECTED)
          if (ip[0] != 0) { 
              state = CONNECTED;
              Serial.println("[WiFi] CONNECTED! (IP Assigned)");
              printWifiStatus();
              connectionAttempts = 0;
          } else {
              // Still waiting for IP, stay in CONNECTING
              // Occasionally print status if needed, or just wait.
              if ((currentMillis % 500) == 0) Serial.print("."); 
          }
      } else if (currentMillis - lastConnectionAttempt > 10000) {
          // Timeout after 10s
          Serial.println("[WiFi] Connection Timeout.");
          state = DISCONNECTED; // Go back to retry
          WiFi.disconnect(); // Clear state
          
          if (connectionAttempts > 5) {
              Serial.println("[WiFi] Too many failed attempts. Entering Offline Mode temporarily.");
              state = OFFLINE;
              lastConnectionAttempt = currentMillis; // Use this as timer for offline duration
          }
      }
      break;

    case OFFLINE:
       // Stay offline for 30 seconds then retry
       if (currentMillis - lastConnectionAttempt > 30000) {
           Serial.println("[WiFi] Retrying connection after offline wait...");
           state = DISCONNECTED;
           connectionAttempts = 0;
       }
       break;

    case CONNECTED:
       // Periodically check if we lost connection (every 5s)
       if (currentMillis - lastConnectionAttempt > 5000) {
           lastConnectionAttempt = currentMillis;
           if (WiFi.status() != WL_CONNECTED) {
               Serial.println("[WiFi] Lost connection!");
               state = DISCONNECTED;
               WiFi.disconnect();
           }
       }
       break;
  }
#endif

  // Only process UDP if Connected
  if (state == CONNECTED || state == OFFLINE) { // Allow AP/Offline logic
      // In OFFLINE we might want to skip UDP, but keeping it safe.
      if (state == CONNECTED) {
        int packetSize = Udp.parsePacket();
        if (packetSize) {
            int len = Udp.read(packetBuffer, 255);
            if (len > 0) packetBuffer[len] = 0;
            lastMessage = String(packetBuffer);
            newMessageAvailable = true;
            // Serial.println("Msg: " + lastMessage);
        }
      }
  }
}    


bool NetworkManager::sendPacket(const String &message) {
  // For carts, we typically broadcast if we don't know the other IP,
  // or we could store the peer IP. For simplicity, we broadcast here
  // or assume a fixed IP scheme if in AP mode.
  // However, 255.255.255.255 works for local subnets usually.

  // Note: IPAddress(255, 255, 255, 255) is the broadcast address

  if (Udp.beginPacket(IPAddress(255, 255, 255, 255), UDP_PORT) == 1) {
    Udp.write(message.c_str());
    Udp.endPacket();
    return true;
  }
  return false;
}

bool NetworkManager::respondToLastSender(const String &message) {
  // Reply directly to the device that sent the last packet
  if (Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()) == 1) {
    Udp.write(message.c_str());
    Udp.endPacket();
    return true;
  }
  return false;
}

void NetworkManager::sendTelemetry(int state, int sensors, float distance) {
  // Formato JSON simple: {"s":state, "v":sensors, "d":dist}
  String json = "{\"s\":" + String(state) + ",\"v\":" + String(sensors) +
                ",\"d\":" + String(distance, 1) + "}";
  sendPacket(json);
}

bool NetworkManager::hasNewMessage() {
  bool temp = newMessageAvailable;
  newMessageAvailable = false;
  return temp;
}

String NetworkManager::getLastMessage() { return lastMessage; }


bool NetworkManager::isConnected() {
    return state == CONNECTED;
}

bool NetworkManager::isConnecting() {
    return state == CONNECTING;
}

void NetworkManager::printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}
