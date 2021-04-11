#include "SpaceshipDisplay.h"

/*
 * Font for 7-segment number sections.
 * Numeration of bits:
 *      0
 *   1     2
 *      3
 *   4     5
 *      6
 */
const uint8_t SpaceshipDisplay::DIGITS[] = {
        0b1110111, // 0
        0b0100100, // 1
        0b1011101, // 2
        0b1101101, // 3
        0b0101110, // 4
        0b1101011, // 5
        0b1111011, // 6
        0b0100101, // 7
        0b1111111, // 8
        0b1101111, // 9
        0b0000000, // 10, blank
        0b0001000, // 11, -
        0b1011011, // 12, E
        0b0111110, // 13, H
        0b0010000, // 14, i
        0b1010010, // 15, L
        0b1111000, // 16, o
        0b1000001, // 17, spinner frame 1
        0b0010100, // 18, spinner frame 2
        0b0100010, // 19, spinner frame 3
};

void SpaceshipDisplay::commitState(msm6775::SegmentDriver &driver) {
    segments.commit(driver);
}

void SpaceshipDisplay::clearAll() {
    segments.writeAll(false);
}

void SpaceshipDisplay::setAll() {
    segments.writeAll(true);
}

void SpaceshipDisplay::renderTwoDigitNumber(uint8_t *digits, int8_t value) {
    if (value >= 100) {
        digits[0] = DIGIT_ERROR;
        digits[1] = DIGIT_ERROR;
    } else if (value >= 0) {
        digits[0] = value / 10;
        digits[1] = value % 10;
    } else if (value > -10) {
        digits[0] = DIGIT_DASH;
        digits[1] = -value % 10;
    } else if (value == NUM_BLANK) {
        digits[0] = DIGIT_BLANK;
        digits[1] = DIGIT_BLANK;
    } else if (value == NUM_DASH) {
        digits[0] = DIGIT_DASH;
        digits[1] = DIGIT_DASH;
    } else if (value == Hvac::TEMP_HI) {
        digits[0] = DIGIT_H;
        digits[1] = DIGIT_I;
    } else if (value == Hvac::TEMP_LO) {
        digits[0] = DIGIT_L;
        digits[1] = DIGIT_O;
    } else if (value >= NUM_SPINNER && value < NUM_SPINNER + NUM_SPINNER_FRAMES) {
        auto digit = DIGIT_SPINNER + (value - NUM_SPINNER);
        digits[0] = digit;
        digits[1] = digit;
    } else {
        digits[0] = DIGIT_ERROR;
        digits[1] = DIGIT_ERROR;
    }
}

const SpaceshipDisplay::Glyph SpaceshipDisplay::Glyph::from(uint32_t data) {
    return SpaceshipDisplay::Glyph{{
            static_cast<uint8_t>(data >> 16),
            static_cast<uint8_t>(data >> 8),
            static_cast<uint8_t>(data)
    }};
}

uint32_t SpaceshipDisplay::Glyph::toUint32() const {
    return ((uint32_t) data[0] << 16) | ((data[1] << 8) | data[2]);
}

const SpaceshipDisplay::Glyph SpaceshipDisplay::Glyph::BLANK = from(0);
