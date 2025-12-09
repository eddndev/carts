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
        matrix.loadFrame(FRAME_IDLE);
        isAnimating = false;
    }
}
