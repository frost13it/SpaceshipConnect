#include <Arduino.h>
#include <avr/parity.h>

#include "SpaceshipHvac.h"

static uint8_t decodeTemp(uint8_t value) {
    switch (value) {
        case 0:
            return 0;
        case 1:
            return SpaceshipHvac::TEMP_LO;
        case 15:
            return SpaceshipHvac::TEMP_HI;
        default:
            return value + 14;
    }
}

bool SpaceshipHvac::readState(State &state) {
    uint8_t frames[PACKET_SIZE];
    if (!readPacket(frames)) {
        return false;
    }

    state.connected = frames[6] != 0;
    state.fanSpeed = (frames[1] >> 1) & 0x7;
    state.airDirection = (frames[2] >> 4) & 0x7;
    state.autoMode = frames[2] & bit(1);
    state.leftWheel = frames[2] & bit(3);
    state.acOn = frames[3] & bit(1);
    state.acOff = frames[3] & bit(2);
    state.temp1 = decodeTemp((frames[5] >> 1) & 0xf);
    state.temp2 = decodeTemp((frames[4] >> 1) & 0xf);
    return true;
}

static bool checkParity(uint8_t value) {
    return parity_even_bit(value);
}

bool SpaceshipHvac::readPacket(uint8_t *frames) {
    for (int i = 0; i < PACKET_SIZE; ++i) {
        frames[i] = readByte();
    }

    // check idle frame
    if (frames[0] != 0) return false;

    // check if HVAC is offline
    bool allZero = true;
    for (int i = 1; i < PACKET_SIZE; ++i) {
        if (frames[i] != 0) {
            allZero = false;
            break;
        }
    }
    if (allZero) return true;

    // check parity bits
    for (int i = 1; i < PACKET_SIZE; ++i) {
        if (!checkParity(frames[i])) return false;
    }

    // check CRC
    uint8_t crc = 0;
    for (int i = 1; i < PACKET_SIZE; ++i) {
        crc ^= frames[i];
    }
    if (crc != 0b10000001) return false;

    return true;
}

uint8_t SpaceshipHvac::readByte() {
    uint8_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= readBit() << i;
    }
    return result;
}

bool SpaceshipHvac::readBit() {
    clockPin.writePulse(CLOCK_PERIOD);
    delayMicroseconds(CLOCK_PERIOD);
    return !dataPin.read();
}
