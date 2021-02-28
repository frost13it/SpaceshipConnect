#include <Arduino.h>

#include "Pin.h"
#include "CriticalSection.h"

Pin::Pin(uint8_t number) {
    uint8_t port = digitalPinToPort(number);
    if (port != NOT_A_PIN) {
        inReg = portInputRegister(port);
        outReg = portOutputRegister(port);
        modeReg = portModeRegister(port);
        mask = digitalPinToBitMask(number);
    } else {
        inReg = nullptr;
        outReg = nullptr;
        modeReg = nullptr;
        mask = 0;
    }
}

static void maskSet(volatile uint8_t *reg, uint8_t mask) {
    *reg |= mask;
}

static void maskClear(volatile uint8_t *reg, uint8_t mask) {
    *reg &= ~mask;
}

static void maskWrite(volatile uint8_t *reg, uint8_t mask, bool value) {
    if (value) {
        maskSet(reg, mask);
    } else {
        maskClear(reg, mask);
    }
}

void Pin::doInput(bool pullUp) const {
    if (!mask) return;
    CriticalSection cs;
    maskWrite(outReg, mask, pullUp);
    maskClear(modeReg, mask);
}

void Pin::doOutput(bool initValue) const {
    if (!mask) return;
    CriticalSection cs;
    maskWrite(outReg, mask, initValue);
    maskSet(modeReg, mask);
}

bool Pin::read() const {
    if (!mask) return false;
    return *inReg & mask;
}

void Pin::writeHigh() const {
    if (!mask) return;
    CriticalSection cs;
    maskSet(outReg, mask);
}

void Pin::writeLow() const {
    if (!mask) return;
    CriticalSection cs;
    maskClear(outReg, mask);
}

void Pin::write(bool value) const {
    if (!mask) return;
    CriticalSection cs;
    maskWrite(outReg, mask, value);
}

void Pin::writePulse() const {
    if (!mask) return;
    CriticalSection cs;
    maskSet(outReg, mask);
    maskClear(outReg, mask);
}

void Pin::writePulse(uint16_t delay) const {
    writeHigh();
    delayMicroseconds(delay);
    writeLow();
}
