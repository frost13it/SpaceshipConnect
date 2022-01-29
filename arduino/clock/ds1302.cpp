#include <Arduino.h>

#include "ds1302.h"

void Ds1302::initPins() {
    pins.clock.doOutput();
    pins.io.doOutput();
    pins.ce.doOutput();
}

Ds1302::Transaction::Transaction(Pins &pins, uint8_t address, bool read) : pins(pins) {
    pins.ce.writeHigh();
    uint8_t command = 1 << 7 | address << 1 | (read ? 1 : 0);
    sendByte(command, !read);
}

Ds1302::Transaction::~Transaction() {
    pins.io.doOutput();
    pins.clock.writeLow();
    pins.ce.writeLow();
}

void Ds1302::Transaction::sendByte(uint8_t value, bool finishClockPulse) {
    for (int i = 7; i >= 0; i--) {
        auto bit = bitRead(value, 0);
        pins.io.write(bit);
        pins.clock.writeHigh();
        if (i > 0 || finishClockPulse) pins.clock.writeLow();
        value >>= 1;
    }
}

uint8_t Ds1302::Transaction::receiveByte() {
    uint8_t result = 0;
    pins.io.doInput();
    for (int i = 0; i < 8; ++i) {
        // may already be high after the last command's bit
        pins.clock.writePulse();
        auto bit = pins.io.read();
        result |= bit << i;
    }
    return result;
}

uint8_t Ds1302::readRegister(uint8_t offset) {
    Transaction io(pins, offset, true);
    return io.receiveByte();
}

void Ds1302::writeRegister(uint8_t offset, uint8_t value) {
    Transaction io(pins, offset, false);
    io.sendByte(value);
}

void Ds1302::readBurst(uint8_t *dest, uint8_t size, bool ram) {
    uint8_t address = BURST_ADDRESS;
    if (ram) {
        size = min(size, RAM_SIZE);
        address |= bit(5);
    } else {
        size = min(size, CLOCK_SIZE);
    }

    Transaction io(pins, address, true);
    while (size > 0) {
        *(dest++) = io.receiveByte();
        size--;
    }
}

void Ds1302::writeBurst(const uint8_t *source, uint8_t size, bool ram) {
    uint8_t address = BURST_ADDRESS;
    if (ram) {
        size = min(size, RAM_SIZE);
        address |= bit(5);
    } else {
        size = min(size, CLOCK_SIZE);
    }

    Transaction io(pins, address, false);
    while (size > 0) {
        io.sendByte(*(source++));
        size--;
    }
}

bool Ds1302::isClockHalt() {
    auto reg = readRegister(0);
    return bitRead(reg, 7);
}

void Ds1302::setClockHalt(bool value) {
    auto reg = readRegister(0);
    auto current = bitRead(reg, 7);
    if (current) {
        bitWrite(reg, 7, value);
        writeRegister(0, reg);
    } else {
        uint8_t data[8];
        readBurst(data, sizeof data, false);
        bitWrite(data[0], 7, value);
        writeBurst(data, sizeof data, false);
    }
}

bool Ds1302::isWriteProtect() {
    return bitRead(readRegister(7), 7);
}

void Ds1302::setWriteProtect(bool value) {
    writeRegister(7, value ? bit(7) : 0);
}

static uint8_t bcdEncode(uint8_t value) {
    return (value / 10) << 4 | (value % 10);
}

static uint8_t bcdDecode(uint8_t value) {
    return (value >> 4) * 10 + (value & 0xF);
}

DateTime Ds1302::getDateTime() {
    uint8_t data[CLOCK_SIZE];
    readBurst(data, sizeof data, false);
    Time time = {
            bcdDecode(data[2]),
            bcdDecode(data[1]),
            bcdDecode(data[0] & 0x7f),
    };
    Date date = {
            bcdDecode(data[5]),
            bcdDecode(data[3]),
            bcdDecode(data[4]),
            bcdDecode(data[6]),
    };
    return DateTime{date, time};
}

void Ds1302::setDateTime(DateTime &dateTime, bool clockHalt, bool writeProtect) {
    uint8_t data[] = {
            (uint8_t) ((clockHalt ? bit(7) : 0) | bcdEncode(dateTime.time.second)),
            bcdEncode(dateTime.time.minute),
            bcdEncode(dateTime.time.hour),
            bcdEncode(dateTime.date.dayOfMonth),
            bcdEncode(dateTime.date.month),
            bcdEncode(dateTime.date.dayOfWeek),
            bcdEncode(dateTime.date.year),
            (uint8_t) (writeProtect ? bit(7) : 0),
    };
    writeBurst(data, sizeof data, false);
}

uint8_t Ds1302::getTrickleChargingConfig() {
    return readRegister(8);
}

uint8_t Ds1302::readRam(uint8_t offset) {
    if (offset > 30) return 0xEE;
    return readRegister(bit(5) | offset);
}

void Ds1302::writeRam(uint8_t offset, uint8_t value) {
    if (offset > 30) return;
    writeRegister(bit(5) | offset, value);
}

void Ds1302::readRam(void *dest, uint8_t size) {
    readBurst(static_cast<uint8_t *>(dest), size, true);
}

void Ds1302::writeRam(const void *source, uint8_t size) {
    writeBurst(static_cast<const uint8_t *>(source), size, true);
}
