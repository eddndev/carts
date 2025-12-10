
![Arduino](https://img.shields.io/badge/-Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
![C++](https://img.shields.io/badge/-C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Flutter](https://img.shields.io/badge/-Flutter-02569B?style=for-the-badge&logo=flutter&logoColor=white)
![Dart](https://img.shields.io/badge/-Dart-0175C2?style=for-the-badge&logo=dart&logoColor=white)

# WiFi Line Follower Carts

Project for two Arduino Uno R4 WiFi based line-following carts with peer-to-peer communication and a mobile control app.

## Structure

- `firmware/` - Arduino source code (C++).
- `cart_controller/` - Mobile Control App (Flutter/Dart).
- `tools/` - Extra scripts.

## Setup

1. **Firmware**: Use `arduino-cli` with `arduino:renesas_uno`.
2. **App**: Use `flutter run` in `cart_controller/`.

## Features

### 🤖 Robot Firmware
- [x] **Line Following**: Robust PID control (Kp=0.09) with deadband compensation.
- [x] **Auto-Calibration**: Sensor threshold detection with LED Matrix feedback.
- [x] **P2P WiFi**: UDP communication mesh.
- [x] **Visuals**: Real-time position tracking on LED Matrix.

### 📱 Mobile Controller
- [x] **UDP Discovery**: Auto-scans local subnet for active robots.
- [x] **Remote Control**: Wireless commands (Explore, Stop, Reset).
- [x] **Cyberpunk UI**: Reactive interface with haptic/visual feedback.

## Configuration (v1.0)
- **Base Speed**: 90
- **PID**: Kp=0.09, Kd=1.0
- **Sensors**: QTR-8A (Analog) on Pins A5-A0.
- **Motors**: L298N Driver.
