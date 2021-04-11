#include <Arduino.h>
#include "msm6775.h"
#include "../util/CriticalSection.h"

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

bool SegmentsState::readSegment(uint16_t index) {
    // reversed for convenience
    index = SEGMENTS_COUNT - 1 - index;
    return bitRead(state[index / 8], index % 8);
}

void SegmentsState::writeSegment(uint16_t index, bool value) {
    // reversed for convenience
    index = SEGMENTS_COUNT - 1 - index;
    bitWrite(state[index / 8], index % 8, value);
}

void SegmentsState::writeAll(bool value) {
    memset(state, value ? 0xFF : 0x00, STATE_SIZE);
}

void SegmentsState::writeFrom(const SegmentsState &other) {
    memcpy(state, other.state, STATE_SIZE);
}

void SegmentsState::commit(SegmentDriver &driver) {
    for (int blockIndex = 0; blockIndex < BLOCKS_COUNT; ++blockIndex) {
        auto blockStateOffset = (size_t) blockIndex * BLOCK_STATE_SIZE;
        driver.write(state + blockStateOffset, BLOCK_STATE_SIZE);

        uint8_t blockMask = 1 << (blockIndex + 3);
        driver.writeByte(blockMask);

        driver.load();
    }
}

Emulator *Emulator::instance = nullptr;

uint16_t Emulator::getStateVersion() {
    // for atomic read of 2 bytes
    CriticalSection cs;
    return stateVersion;
}

void Emulator::configureHardware(uint8_t loadInterruptPin) {
    auto interrupt = digitalPinToInterrupt(loadInterruptPin);
    if (interrupt == NOT_AN_INTERRUPT) {
        return;
    }
    pinMode(loadInterruptPin, INPUT);
    attachInterrupt(interrupt, onLoad_ISR, RISING);

    // disable master
    bitClear(SPCR, MSTR);
    // turn on interrupts
    bitSet(SPCR, SPIE);
    // set LSB first
    bitSet(SPCR, DORD);
    // turn on SPI
    bitSet(SPCR, SPE);
}

void Emulator::onLoad_ISR() {
    instance->onLoad();
}

static void copyVolatile(volatile uint8_t *dest, const volatile uint8_t *src, int8_t n) {
    while (n--) {
        *dest++ = *src++;
    }
}

void Emulator::onLoad() {
    if (bitRead(SPSR, SPIF)) {
        // SPI transfer finish ISR is pending
        // clear the flag and run it
        bitClear(SPSR, SPIF);
        onSpiTransferFinished();
    }

    auto startOffset = packetPosition;
    packetPosition = 0;

    uint8_t lastIndex = (startOffset ? startOffset : PACKET_SIZE) - 1;
    uint8_t mask = packet[lastIndex] >> 3 & ALL_BLOCKS_MASK;
    uint8_t blockIndex = 0;
    while (mask) {
        if (mask & 1) {
            auto blockOffset = blockIndex * BLOCK_STATE_SIZE;
            auto block = segments.state + blockOffset;
            if (startOffset) {
                auto headLen = PACKET_SIZE - startOffset;
                copyVolatile(block + headLen, packet, startOffset - 1);
                copyVolatile(block, packet + startOffset, headLen);
            } else {
                copyVolatile(block, packet, BLOCK_STATE_SIZE);
            }
        }
        blockIndex++;
        mask >>= 1;
    }
    stateVersion++;
}

void Emulator::onSpiTransferFinished() {
    uint8_t nextByte = SPDR;
    uint8_t pos = packetPosition;
    packet[pos] = nextByte;
    if (++pos == PACKET_SIZE) pos = 0;
    packetPosition = pos;
}
