/*
 * WiFi Scanner
 * 
 * Upload this sketch to the Arduino Uno R4 WiFi to see
 * what networks are visible in the environment.
 * 
 * 1. Upload this sketch.
 * 2. Open Serial Monitor (115200 baud).
 * 3. Observe the list of networks and their Signal Strength (RSSI).
 */

#include "WiFiS3.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect.
  }

  Serial.println("Initializing WiFi Scanner...");
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication failed: WiFi module not found!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  Serial.print("Firmware Version: ");
  Serial.println(fv);
}

void loop() {
  Serial.println("Scanning available networks...");
  
  // scanNetworks() returns the number of networks found
  int numSsid = WiFi.scanNetworks();
  
  if (numSsid == -1) {
    Serial.println("Couldn't get a wifi connection");
  } else {
    Serial.print("Number of available networks: ");
    Serial.println(numSsid);

    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
      Serial.print(thisNet);
      Serial.print(") ");
      Serial.print(WiFi.SSID(thisNet));
      Serial.print("\tSignal: ");
      Serial.print(WiFi.RSSI(thisNet));
      Serial.print(" dBm");
      Serial.print("\tEncryption: ");
      printEncryptionType(WiFi.encryptionType(thisNet));
    }
  }
  
  Serial.println("\n-------------------------------------\n");
  // Wait 5 seconds before scanning again
  delay(5000);
}

void printEncryptionType(int thisType) {
  switch (thisType) {
    case ENC_TYPE_WEP:
      Serial.println("WEP");
      break;
    case ENC_TYPE_WPA:
      Serial.println("WPA");
      break;
    case ENC_TYPE_WPA2:
      Serial.println("WPA2");
      break;
    case ENC_TYPE_WPA3:
      Serial.println("WPA3");
      break;
    case ENC_TYPE_NONE:
      Serial.println("None");
      break;
    case ENC_TYPE_AUTO:
      Serial.println("Auto");
      break;
    case ENC_TYPE_UNKNOWN:
    default:
      Serial.println("Unknown");
      break;
  }
}
