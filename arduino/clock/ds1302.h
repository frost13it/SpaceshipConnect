#pragma once

#include <stdint.h>
#include <stddef.h>

#include "DateTime.h"
#include "../util/Pin.h"

class Ds1302 {

    struct Pins {
        Pin clock;
        Pin io;
        Pin ce;

        Pins(uint8_t clock, uint8_t io, uint8_t ce) : clock(clock), io(io), ce(ce) { };

    };

    Pins pins;

    static const uint8_t CLOCK_SIZE = 8;
    static const uint8_t RAM_SIZE = 31;
    static const uint8_t BURST_ADDRESS = 31;

    void initPins();

    uint8_t readRegister(uint8_t offset);

    void writeRegister(uint8_t offset, uint8_t value);

    void readBurst(uint8_t *dest, uint8_t size, bool ram);

    void writeBurst(const uint8_t *source, uint8_t size, bool ram);

    class Transaction {

        Pins &pins;

    public:

        Transaction(Pins &pins, uint8_t address, bool read);

        ~Transaction();

        void sendByte(uint8_t value, bool finishClockPulse = true);

        uint8_t receiveByte();

    };

public:

    Ds1302(uint8_t clockPin, uint8_t ioPin, uint8_t cePin) : pins(clockPin, ioPin, cePin) {
        initPins();
    }

    bool isClockHalt();

    void setClockHalt(bool value);

    bool isWriteProtect();

    void setWriteProtect(bool value);

    DateTime getDateTime();

    void setDateTime(DateTime &dateTime, bool clockHalt = false, bool writeProtect = false);

    uint8_t getTrickleChargingConfig();

    uint8_t readRam(uint8_t offset);

    void readRam(void *dest, uint8_t size);

    void writeRam(uint8_t offset, uint8_t value);

    void writeRam(const void *source, uint8_t size);

};
