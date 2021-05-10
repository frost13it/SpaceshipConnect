#include <Arduino.h>

#include "ButtonSet.h"

ButtonSet::ButtonSet(uint8_t pinNumber, uint8_t tolerance, uint8_t levelsCount, const uint16_t *levels)
        : pinNumber(pinNumber),
          tolerance(tolerance),
          levelsCount(levelsCount),
          levels(levels) {
    pinMode(pinNumber, INPUT);
}

uint8_t ButtonSet::getPressedButton() {
    auto rawButton = getRawPressedButton();
    if (rawButton == 0 || rawButton > 0 && pressedButton == 0) {
        pressedButton = rawButton;
    }
    return pressedButton;
}

int8_t ButtonSet::getRawPressedButton() {
    auto adcValue = analogRead(pinNumber);
    for (uint8_t i = 0; i < levelsCount; ++i) {
        auto candidateValue = pgm_read_word(levels + i);
        if (abs(adcValue - (int16_t) candidateValue) <= tolerance) {
            return i;
        }
    }
    return -1;
}
