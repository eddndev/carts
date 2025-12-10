#include "NetworkManager.h"

NetworkManager::NetworkManager() {
    newMessageAvailable = false;
    lastPingTime = 0;
}

void NetworkManager::begin() {
    // Check for WiFi module
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication failed: WiFi module not found!");
        while (true); // Stop here
    }

    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Please upgrade the firmware");
    }

#if IS_ACCESS_POINT
    Serial.print("Creating Access Point: ");
    Serial.println(SECRET_SSID);
    
    // Create open network if no password, or secured if password provided
    IPAddress local_ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    
    WiFi.config(local_ip, gateway, subnet);
    
    if (String(SECRET_PASS).length() == 0) {
        WiFi.beginAP(SECRET_SSID);
    } else {
        WiFi.beginAP(SECRET_SSID, SECRET_PASS);
    }
#else
    Serial.print("Connecting to: ");
    Serial.println(SECRET_SSID);
    
    // Attempt to connect to WiFi network
    // Attempt to connect to WiFi network
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(SECRET_SSID, SECRET_PASS);
        Serial.print(".");
        // Check for connection every 500ms for 10 seconds, then try again
        for(int i=0; i<20; i++) {
             if (WiFi.status() == WL_CONNECTED) break;
             delay(250); 
        }
        attempts++;
        if (attempts > 5) {
             Serial.println("WiFi Retry...");
             // Maybe show error on LED?
        }
    }
    Serial.println("\nConnected to WiFi");
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

bool NetworkManager::sendPacket(const String& message) {
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

bool NetworkManager::hasNewMessage() {
    bool temp = newMessageAvailable;
    newMessageAvailable = false;
    return temp;
}

String NetworkManager::getLastMessage() {
    return lastMessage;
}

void NetworkManager::printWifiStatus() {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}
