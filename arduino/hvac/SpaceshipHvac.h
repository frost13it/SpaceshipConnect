#pragma once

#include "../util/Pin.h"

class SpaceshipHvac {

    Pin clockPin;
    Pin dataPin;

    static const uint8_t PACKET_SIZE = 7;
    static const uint16_t CLOCK_PERIOD = 50;

    bool readPacket(uint8_t *frames);

    uint8_t readByte();

    bool readBit();

public:

    SpaceshipHvac(uint8_t _clockPin, uint8_t _dataPin) : clockPin(_clockPin), dataPin(_dataPin) {
        clockPin.doOutput();
        dataPin.doInput(true);
    };

    static const uint8_t TEMP_LO = 1;
    static const uint8_t TEMP_HI = 255;

    struct State {
        bool connected;
        uint8_t fanSpeed;
        uint8_t airDirection;
        bool autoMode;
        bool acOn;
        bool acOff;
        bool leftWheel;
        uint8_t temp1;
        uint8_t temp2;
    };

    bool readState(State &state);

};
