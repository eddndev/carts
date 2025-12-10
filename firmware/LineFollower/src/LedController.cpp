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
    if (isAnimating && (millis() - lastAnimationTime > 2000)) {
         // Auto-clear after 2 seconds only for PING. 
         // For STOP/EXPLORE we might want to keep it?
         // Let's assume the main loop calls 'showLine' which clears isAnimating
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

void LedController::showCalibration() {
   // Simple full fill or blinking pattern
   // Let's do a static "Calibrating" pattern (Outer box)
   const uint32_t FRAME_CALIB[] = {
        0xFFF00000,
        0x80180180,
        0x00000FFF
   };
   matrix.loadFrame(FRAME_CALIB);
}

void LedController::showLinePosition(uint16_t position) {
    if (isAnimating) return; 

    // Position is 0 to (Count-1)*1000.  For 6 sensors: 0-5000.
    // Map 0-5000 to length of bar (0-12 columns)
    
    int barWidth = map(position, 0, 5000, 0, 12);
    if (barWidth < 0) barWidth = 0;
    if (barWidth > 12) barWidth = 12;
    
    // Serial.print("Pos: "); Serial.print(position); Serial.print(" Width: "); Serial.println(barWidth); // UNCOMMENT FOR DEBUG
    Serial.print("Pos: "); Serial.print(position); Serial.print(" Width: "); Serial.println(barWidth);
    
    // Draw columns from 0 up to barWidth
    uint8_t frame[8][12] = {0};
    
    for (int col = 0; col < 12; col++) {
        if (col < barWidth) {
             // Fill this column
             for (int r = 0; r < 8; r++) {
                 frame[r][col] = 1;
             }
        }
    }
    
    matrix.renderBitmap(frame, 8, 12);
}


void LedController::showStop() {
    // Big X 
    uint8_t frame[8][12] = {
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {0,1,0,0,0,0,0,0,0,0,1,0},
        {0,0,1,0,0,0,0,0,0,1,0,0},
        {0,0,0,1,0,0,0,0,1,0,0,0},
        {0,0,0,1,0,0,0,0,1,0,0,0},
        {0,0,1,0,0,0,0,0,0,1,0,0},
        {0,1,0,0,0,0,0,0,0,0,1,0},
        {1,0,0,0,0,0,0,0,0,0,0,1}
    };
    matrix.renderBitmap(frame, 8, 12);
    isAnimating = true;
    lastAnimationTime = millis();
}

void LedController::showExplore() {
    // WiFi / Radar Symbol
    uint8_t frame[8][12] = {
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,1,1,1,1,1,1,0,0,0},
        {0,0,1,0,0,0,0,0,0,1,0,0},
        {0,1,0,0,0,0,0,0,0,0,1,0},
        {0,1,0,0,0,1,1,0,0,0,1,0},
        {1,0,0,0,1,1,1,1,0,0,0,1},
        {0,0,0,0,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,1,1,0,0,0,0,0}
    };
    matrix.renderBitmap(frame, 8, 12);
    isAnimating = true;
    lastAnimationTime = millis();
}

void LedController::showReset() {
    // Full Fill
    uint8_t frame[8][12];
    for(int r=0; r<8; r++) {
        for(int c=0; c<12; c++) frame[r][c] = 1;
    }
    matrix.renderBitmap(frame, 8, 12);
    isAnimating = true;
    lastAnimationTime = millis();
}

void LedController::showPacketReceived() {
    // A small square or dot in the corner to indicate data
    uint8_t frame[8][12] = {0};
    // Draw a box in top right corner (Row 0, Col 10-11)
    frame[0][10] = 1; frame[0][11] = 1;
    frame[1][10] = 1; frame[1][11] = 1;
    
    matrix.renderBitmap(frame, 8, 12);
    isAnimating = true;
    lastAnimationTime = millis(); 
}
