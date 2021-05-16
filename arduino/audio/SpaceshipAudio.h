#pragma once

#include "../display/SpaceshipDisplay.h"

class SpaceshipAudio {

    SpaceshipDisplay display;
    msm6775::Emulator displayEmulator;

    bool _isSwitchedOn = false;
    bool _isAuxDisplayed = false;

public:

    SpaceshipAudio(uint8_t dispCePin) : display(), displayEmulator(display.segments, dispCePin) {}

    const SpaceshipDisplay &getDisplay() {
        return display;
    }

    void onSpiTransferFinished();

    void refreshState();

    bool isSwitchedOn();

    bool isAuxModeDisplayed();

};


