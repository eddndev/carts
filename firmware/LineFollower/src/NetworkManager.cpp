#include "NetworkManager.h"

NetworkManager::NetworkManager() {
  newMessageAvailable = false;
  lastPingTime = 0;
}

void NetworkManager::begin() {
  // Check for WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication failed: WiFi module not found!");
    while (true)
      ; // Stop here
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

#if IS_ACCESS_POINT
  Serial.println("[WiFi] === ACCESS POINT MODE ===");
  Serial.print("[WiFi] Creating network: ");
  Serial.println(SECRET_SSID);

  // Create open network if no password, or secured if password provided
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.config(local_ip, gateway, subnet);

  int apStatus;
  if (String(SECRET_PASS).length() == 0) {
    apStatus = WiFi.beginAP(SECRET_SSID);
  } else {
    apStatus = WiFi.beginAP(SECRET_SSID, SECRET_PASS);
  }
  
  // Wait for AP to be ready
  delay(1000);
  
  if (apStatus == WL_AP_LISTENING) {
    Serial.println("[WiFi] Access Point ACTIVE!");
    Serial.print("[WiFi] AP IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("[WiFi] Waiting for clients to connect...");
  } else {
    Serial.print("[WiFi] AP creation FAILED. Status: ");
    Serial.println(apStatus);
  }
#else
  Serial.print("[WiFi] Connecting to SSID: ");
  Serial.println(SECRET_SSID);

  // Attempt to connect to WiFi network
  int attempts = 0;
  int maxAttempts = 20;
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    attempts++;
    Serial.print("[WiFi] Attempt ");
    Serial.print(attempts);
    Serial.print("/");
    Serial.print(maxAttempts);
    Serial.print(" - ");
    
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    
    // Wait up to 5 seconds for this attempt
    for (int i = 0; i < 20; i++) {
      delay(250);
      int status = WiFi.status();
      if (status == WL_CONNECTED) {
        break;
      }
    }
    
    // Print detailed status
    int status = WiFi.status();
    switch (status) {
      case WL_CONNECTED:
        Serial.println("SUCCESS!");
        break;
      case WL_NO_SSID_AVAIL:
        Serial.println("FAILED: Network not found (check SSID)");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("FAILED: Connection rejected (check password)");
        break;
      case WL_IDLE_STATUS:
        Serial.println("IDLE: Still trying...");
        break;
      case WL_DISCONNECTED:
        Serial.println("DISCONNECTED: Retrying...");
        break;
      default:
        Serial.print("UNKNOWN STATUS: ");
        Serial.println(status);
        break;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] === CONNECTION SUCCESSFUL ===");
    Serial.print("[WiFi] Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("[WiFi] === CONNECTION FAILED ===");
    Serial.println("[WiFi] Could not connect after maximum attempts.");
    Serial.println("[WiFi] Possible causes:");
    Serial.println("[WiFi]   1. SSID incorrect or network out of range");
    Serial.println("[WiFi]   2. Password incorrect");
    Serial.println("[WiFi]   3. Router/AP is offline");
    Serial.println("[WiFi] The cart will continue but network features disabled.");
  }
#endif

  printWifiStatus();

  // Start UDP
  Udp.begin(UDP_PORT);
  Serial.print("Listening on UDP port: ");
  Serial.println(UDP_PORT);
}

void NetworkManager::update() {
  // Check for packets
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // Read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }

    lastMessage = String(packetBuffer);
    newMessageAvailable = true;
    Serial.println("Contents: " + lastMessage);
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

void NetworkManager::printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}
