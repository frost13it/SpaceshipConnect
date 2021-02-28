#pragma once

#include <stdint.h>

class Pin {

    volatile uint8_t *inReg;
    volatile uint8_t *outReg;
    volatile uint8_t *modeReg;
    uint8_t mask;

public:

    explicit Pin(uint8_t number);

    void doInput(bool pullUp = false) const;

    void doOutput(bool initValue = false) const;

    bool read() const;

    void write(bool value) const;

    void writeHigh() const;

    void writeLow() const;

    void writePulse() const;

    void writePulse(uint16_t delay) const;

};
