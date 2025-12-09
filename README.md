
# WiFi Line Follower Carts

Project for two Arduino Uno R4 WiFi based line-following carts with peer-to-peer communication.

## Structure

- `firmware/` - Contains the Arduino source code.
  - `LineFollower/` - Main firmware sketch.
    - `src/` - Modular source files (classes/libraries local to the project).
    - `LineFollower.ino` - Entry point.
- `tools/` - (Optional) Scripts or extra tools.

## Setup

1. This project uses `arduino-cli` for management.
2. Board: Arduino Uno R4 WiFi (`arduino:renesas_uno`).

## Features
- [x] **Line Following**: Robust PID control with deadband compensation and speed matching.
- [x] **Sensor Calibration**: Auto-calibration routine with LED Matrix feedback.
- [x] **WiFi Communication**: Basic P2P UDP packet sending.
- [x] **Status Display**: Real-time line position and calibration status on LED Matrix.

## Configuration (v1.0)
- **Base Speed**: 90
- **PID**: Kp=0.09, Kd=1.0
- **Sensors**: QTR-8A (Analog) on Pins A5-A0.
- **Motors**: L298N Driver.
