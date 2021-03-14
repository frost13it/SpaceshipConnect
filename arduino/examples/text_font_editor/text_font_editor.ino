#include "../../display/SpaceshipDisplay.h"
#include "../../environment.h"

msm6775::SegmentDisplay segmentDisplay(DISPLAY_CLOCK, DISPLAY_DATA, DISPLAY_CE);
SpaceshipDisplay display(segmentDisplay);

void setup() {
    Serial.begin(SERIAL_SPEED);
    display.text.showString(0, "Honda Civic");
    display.update();
    delay(2000);
}

static int BLINK_DELAY = 250;

SpaceshipDisplay::Glyph glyphs[SpaceshipDisplay::Text::SIZE] = {SpaceshipDisplay::Glyph::BLANK};
int cursorPosition = 0;
bool highlightCurrent = false;
int blinkDelay = BLINK_DELAY;
bool charInput = false;

void dumpByte(uint8_t byte, uint8_t len) {
    Serial.print("0b");
    for (int i = len - 1; i >= 0; --i) {
        Serial.print(byte >> i & 1);
    }
    Serial.print(", ");
}

void loop() {
    display.text.clear();
    display.media.mp3(charInput);
    display.media.wma(!charInput);
    for (int i = 0; i < SpaceshipDisplay::Text::SIZE; ++i) {
        auto glyph = glyphs[i];
        if (i == cursorPosition && highlightCurrent) {
            glyph = SpaceshipDisplay::Glyph::from(~glyph.toUint32());
        }
        display.text.showGlyph(i, glyph);
    }
    display.update();
    delay(10);
    blinkDelay -= 10;
    if (blinkDelay <= 0) {
        blinkDelay = BLINK_DELAY;
        highlightCurrent = !highlightCurrent;
    }

    int command = Serial.read();
    if (command == -1) {
        return;
    } else if (command >= '0' && command <= '9') {
        switch (command - '0') {
            case 1:
                cursorPosition = (cursorPosition + SpaceshipDisplay::Text::SIZE - 1) % SpaceshipDisplay::Text::SIZE;
                return;
            case 2:
                cursorPosition = (cursorPosition + 1) % SpaceshipDisplay::Text::SIZE;
                return;
            case 3: {
                auto glyph = glyphs[cursorPosition];
                dumpByte(glyph.data[0], 3);
                dumpByte(glyph.data[1], 8);
                dumpByte(glyph.data[2], 8);
                Serial.println();
                return;
            }
            case 4:
                glyphs[cursorPosition] = SpaceshipDisplay::Glyph::BLANK;
                return;
            case 5:
                charInput = !charInput;
                return;
        }
    } else if (charInput) {
        glyphs[cursorPosition] = SpaceshipDisplay::Text::getGlyph(command);
        cursorPosition = (cursorPosition + 1) % SpaceshipDisplay::Text::SIZE;
        return;
    } else if (command >= 'a' && command < 'a' + 19) {
        auto bit = command - 'a';
        auto glyphData = glyphs[cursorPosition].toUint32();
        bitToggle(glyphData, bit);
        glyphs[cursorPosition] = SpaceshipDisplay::Glyph::from(glyphData);
        return;
    }
    Serial.print(F("Unknown command: "));
    Serial.println(command);
}
