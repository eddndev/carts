#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "Config.h"
#include <Arduino.h>
#include <WiFiS3.h>


class NetworkManager {
public:
  NetworkManager();
  void begin();
  void update(); // Call this in loop()
  bool sendPacket(const String &message);
  bool respondToLastSender(const String &message);
  String getLastMessage();
  bool hasNewMessage();
  void sendTelemetry(int nodeId, int sensorState, float distance);

private:
  WiFiUDP Udp;
  char packetBuffer[255];
  String lastMessage;
  bool newMessageAvailable;
  unsigned long lastPingTime;

  void printWifiStatus();
};

#endif
