#include <Arduino.h>
#include "../../environment.h"
#include "../../util/Pin.h"
#include "../../display/SpaceshipDisplay.h"

SpaceshipDisplay display;
msm6775::Emulator emulator(display.segments, DISPLAY_EMULATOR_CE);
SpaceshipDisplay::Glyph glyphs[SpaceshipDisplay::Text::SIZE];

const bool DUMP_UNRECOGNIZED = false;

void setup() {
    Serial.begin(SERIAL_SPEED);
}

char recognizeGlyph(SpaceshipDisplay::Glyph &glyph) {
    for (char c = 32; c < 127; ++c) {
        auto candidate = SpaceshipDisplay::Text::getFontGlyph(c);
        if (!memcmp(&candidate, &glyph, sizeof candidate)) return c;
    }
    return '\x1A';
}

void loop() {
    delay(49);
    SpaceshipDisplay::Glyph newGlyphs[SpaceshipDisplay::Text::SIZE];
    display.text.getGlyphs(newGlyphs);
    auto version = emulator.getStateVersion();
    if (!memcmp(glyphs, newGlyphs, sizeof glyphs)) return;
    memcpy(glyphs, newGlyphs, sizeof glyphs);

    Serial.print(millis());
    Serial.print(": '");
    for (uint8_t i = 0; i < SpaceshipDisplay::Text::SIZE; ++i) {
        auto &glyph = glyphs[i];
        auto c = recognizeGlyph(glyph);
        if (c == '\x1A' && DUMP_UNRECOGNIZED) {
            Serial.print("<");
            Serial.print(glyph.toUint32(), 2);
            Serial.print(">");
        } else {
            Serial.print(c);
        }
    }
    Serial.print("' (");
    Serial.print("version: ");
    Serial.print(version);
    Serial.print(")");
    Serial.println();
}

// SPI interrupt routine
ISR (SPI_STC_vect) {
    emulator.onSpiTransferFinished();
}
