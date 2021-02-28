#include "SpaceshipDisplay.h"

typedef SpaceshipDisplay::Hvac Hvac;

void Hvac::clear() {
    leftTemp(NUM_BLANK, false);
    leftAuto(false);
    rightTemp(NUM_BLANK, false);
    rightAuto(false);
    rightFull(false);
    fan(false, 0);
    ac(false, false);
    mode(false, false, false, false);
}

void Hvac::writeTemp(uint8_t offset, bool frame, int8_t value) {
    uint8_t digits[2];
    renderTwoDigitNumber(digits, value);
    // remove leading zero
    if (digits[0] == 0) digits[0] = DIGIT_BLANK;
    writeTempDigit(offset, digits[0]);
    writeTempDigit(2 + offset, digits[1]);
    segments.writeSegment(2 + offset, frame);
}

void Hvac::writeTempDigit(uint8_t offset, uint8_t value) {
    auto mask = pgm_read_byte(DIGITS + value);
    segments.writeSegment(241 + offset, mask & 1);
    segments.writeSegment(240 + offset, mask & 1 << 1);
    segments.writeSegment(161 + offset, mask & 1 << 2);
    segments.writeSegment(160 + offset, mask & 1 << 3);
    segments.writeSegment(80 + offset, mask & 1 << 4);
    segments.writeSegment(81 + offset, mask & 1 << 5);
    segments.writeSegment(1 + offset, mask & 1 << 6);
}

void Hvac::leftTemp(int8_t value) {
    leftTemp(value, value != NUM_BLANK);
}

void Hvac::leftTemp(int8_t value, bool frame) {
    writeTemp(0, frame, value);
}

void Hvac::leftAuto(bool value) {
    segments.writeSegment(0, value);
}

void Hvac::rightTemp(int8_t value) {
    rightTemp(value, value != NUM_BLANK);
}

void Hvac::rightTemp(int8_t value, bool frame) {
    writeTemp(8, frame, value);
}

void Hvac::rightAuto(bool value) {
    segments.writeSegment(8, value);
}

void Hvac::rightFull(bool value) {
    segments.writeSegment(7, value);
}

void Hvac::fan(bool icon, uint8_t speed) {
    segments.writeSegment(4, icon);
    segments.writeSegment(84, speed >= 1);
    segments.writeSegment(164, speed >= 2);
    segments.writeSegment(244, speed >= 3);
    segments.writeSegment(245, speed >= 4);
    segments.writeSegment(165, speed >= 5);
    segments.writeSegment(85, speed >= 6);
    segments.writeSegment(5, speed >= 7);
}

void Hvac::ac(bool on, bool off) {
    segments.writeSegment(246, on);
    segments.writeSegment(166, off);
}

void Hvac::mode(bool windshield, bool human, bool front, bool bottom) {
    segments.writeSegment(86, windshield);
    segments.writeSegment(87, human);
    segments.writeSegment(247, front);
    segments.writeSegment(167, bottom);
}
