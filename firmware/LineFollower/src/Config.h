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

#endif
