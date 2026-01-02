#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "Config.h"
#include <Arduino.h>
#include <WiFiS3.h>


class NetworkManager {
public:
  enum ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    OFFLINE
  };

  NetworkManager();
  void begin(); // Now non-blocking
  void update(); // Handles state machine
  bool sendPacket(const String &message);
  bool respondToLastSender(const String &message);
  String getLastMessage();
  bool hasNewMessage();
  void sendTelemetry(int nodeId, int sensorState, float distance);
  
  bool isConnected();
  bool isConnecting();

private:
  WiFiUDP Udp;
  char packetBuffer[255];
  String lastMessage;
  bool newMessageAvailable;
  unsigned long lastPingTime;
  
  ConnectionState state;
  unsigned long lastConnectionAttempt;
  int connectionAttempts;

  void checkConnection();
  void printWifiStatus();
};

#endif
