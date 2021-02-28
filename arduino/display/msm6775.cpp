#include <Arduino.h>
#include "msm6775.h"

using namespace msm6775;

void SegmentDriver::configurePins() {
    clockPin.doOutput();
    dataPin.doOutput();
    loadPin.doOutput();
}

void SegmentDriver::writeByte(uint8_t value) {
    for (int i = 0; i < 8; ++i) {
        dataPin.write(value & 1);
        clockPin.writePulse();
        value >>= 1;
    }
}

void SegmentDriver::write(uint8_t *data, size_t size) {
    auto end = data + size;
    while (data < end) {
        writeByte(*data++);
    }
}

void SegmentDriver::load() {
    loadPin.writePulse();
}

void SegmentDisplay::writeSegment(uint16_t index, bool value) {
    // reversed for convenience
    index = SEGMENTS_COUNT - 1 - index;
    bitWrite(state[index / 8], index % 8, value);
}

void SegmentDisplay::writeAll(bool value) {
    memset(state, value ? 0xFF : 0x00, STATE_SIZE);
}

void SegmentDisplay::commitState() {
    for (int blockIndex = 0; blockIndex < BLOCKS_COUNT; ++blockIndex) {
        auto blockStateOffset = (size_t) blockIndex * BLOCK_STATE_SIZE;
        driver.write(state + blockStateOffset, BLOCK_STATE_SIZE);

        uint8_t blockMask = 1 << (blockIndex + 3);
        driver.writeByte(blockMask);

        driver.load();
    }
}
