#include "SpaceshipDisplay.h"

typedef SpaceshipDisplay::Clock Clock;

void Clock::clear() {
    hour(NUM_BLANK);
    colon(false);
    minute(NUM_BLANK);
}

void Clock::hour(int8_t value) {
    uint8_t firstDigit;
    uint8_t secondDigit;
    if (value >= 0 && value <= 29) {
        firstDigit = value / 10;
        secondDigit = value % 10;
    } else if (value == NUM_BLANK) {
        // Clear segments
        firstDigit = 0;
        secondDigit = DIGIT_BLANK;
    } else if (value == NUM_DASH) {
        // Show a dash
        firstDigit = 0;
        secondDigit = DIGIT_DASH;
    } else {
        // Invalid value
        firstDigit = DIGIT_ERROR;
        secondDigit = DIGIT_ERROR;
    }
    writeFirstDigit(firstDigit);
    writeFullDigit(0, secondDigit);
}

void Clock::colon(bool value) {
    segments.writeSegment(282, value);
}

void Clock::minute(int8_t value) {
    uint8_t digits[2];
    renderTwoDigitNumber(digits, value);
    // fixup second digit when a dash is requested
    if (digits[0] == DIGIT_DASH) digits[1] = DIGIT_BLANK;
    writeFullDigit(2, digits[0]);
    writeFullDigit(4, digits[1]);
}

void Clock::writeFirstDigit(uint8_t value) {
    segments.writeSegment(40, value == 1 || value == 2);
    segments.writeSegment(120, value != 0 && value != 2);
    segments.writeSegment(200, value > 1);
}

void Clock::writeFullDigit(uint8_t offset, uint8_t value) {
    uint8_t mask = pgm_read_byte(DIGITS + value);
    segments.writeSegment(41 + offset, mask & 1);
    segments.writeSegment(121 + offset, mask & 1 << 1);
    segments.writeSegment(42 + offset, mask & 1 << 2);
    segments.writeSegment(122 + offset, mask & 1 << 3);
    segments.writeSegment(201 + offset, mask & 1 << 4);
    segments.writeSegment(202 + offset, mask & 1 << 5);
    segments.writeSegment(281 + offset, mask & 1 << 6);
}
