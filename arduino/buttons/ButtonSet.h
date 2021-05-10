#pragma once

#include <stdint.h>

class ButtonSet {

    const uint8_t pinNumber;
    const uint8_t tolerance;
    const uint8_t levelsCount;
    const uint16_t *levels;

    uint8_t pressedButton = 0;

    int8_t getRawPressedButton();

public:

    ButtonSet(uint8_t pinNumber, uint8_t tolerance, uint8_t levelsCount, const uint16_t *levels);

    uint8_t getPressedButton();

};
