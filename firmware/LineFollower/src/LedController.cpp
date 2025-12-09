#include "LedController.h"

LedController::LedController() {
    isAnimating = false;
    lastAnimationTime = 0;
}

void LedController::begin() {
    matrix.begin();
    matrix.loadFrame(FRAME_IDLE);
}

void LedController::showPing() {
    matrix.loadFrame(FRAME_PING);
    lastAnimationTime = millis();
    isAnimating = true;
}

void LedController::update() {
    // Keep the PING frame for 200ms, then revert to IDLE
    if (isAnimating && (millis() - lastAnimationTime > 200)) {
        // matrix.loadFrame(FRAME_IDLE); // Don't revert to IDLE if we are streaming sensor data
        isAnimating = false;
    }
}

void LedController::showSensorValues(uint16_t* values, uint8_t count) {
    if (isAnimating) return; // Don't interrupt ping animation
    
    // Create a temporary frame
    uint8_t frame[8][12] = {0}; 
    
    // Map 6 sensors to 12 columns (2 width each) or 6 columns with spacing
    // Let's use 6 bars of 2 pixel width.
    // Sensors values are 0-1000 (usually) or 0-2500 depending on QTR config.
    // Analog raw is 0-1023.
    
    for (int i = 0; i < count && i < 6; i++) {
        // Normalize value (0-1023) to height (0-8)
        // Inverted: Dark line = high value (~1000), Light = low (~0)
        int height = map(values[i], 0, 1023, 0, 8);
        if (height > 8) height = 8;
        if (height < 0) height = 0;
        
        // Fill columns
        int col = i * 2;
        for (int h = 0; h < height; h++) {
             // Fill from bottom up (row 7 is bottom)
             frame[7-h][col] = 1;
             frame[7-h][col+1] = 1;
        }
    }
    
    matrix.renderBitmap(frame, 8, 12);
}
