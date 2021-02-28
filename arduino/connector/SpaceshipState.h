#pragma once

#include "clock/DateTime.h"
#include "hvac/SpaceshipHvac.h"

struct SpaceshipState {
    SpaceshipHvac::State hvac;
    int8_t temp;
    bool reversing;
};
