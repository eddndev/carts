#ifndef FRAMES_H
#define FRAMES_H

#include <Arduino.h>

// 12x8 LED Matrix Frames
// Each frame is an array of 3 uint32_t values

// Frame: Small Dot (Idle)
const uint32_t FRAME_IDLE[] = {
    0x00000000,
    0x00060000,
    0x00000000
};

// Frame: X (Packet Received)
const uint32_t FRAME_PING[] = {
    0x30300003,
    0x03303030,
    0x00030003
};

// Frame: Heart (Alternate option)
const uint32_t FRAME_HEART[] = {
    0x3184a444,
    0x44042081,
    0x100a0040
};

#endif
