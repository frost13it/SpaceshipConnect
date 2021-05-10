#pragma once

#include "clock/DateTime.h"
#include "hvac/SpaceshipHvac.h"

struct SpaceshipState {
    SpaceshipHvac::State hvac;
    uint8_t audioButton;
    uint8_t hftButton;
    int8_t temp;
    bool reversing;
};
