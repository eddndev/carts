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

## Features (Planned)
- Line following using IR sensors.
- Inter-cart communication via WiFi (UDP).
- Status display on built-in LED Matrix.
