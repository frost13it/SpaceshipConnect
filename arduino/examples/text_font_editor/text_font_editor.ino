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

uint32_t glyphs[SpaceshipDisplay::Text::SIZE] = {0};
int cursorPosition = 0;
bool highlightCurrent = false;
int blinkDelay = BLINK_DELAY;
bool charInput = false;

void dumpByte(uint32_t word, uint8_t shift, uint8_t len) {
    Serial.print("0b");
    for (int i = len - 1; i >= 0; --i) {
        Serial.print(word >> (shift + i) & 1);
    }
    Serial.print(", ");
}

void loop() {
    display.text.clear();
    display.media.mp3(charInput);
    display.media.wma(!charInput);
    for (int i = 0; i < SpaceshipDisplay::Text::SIZE; ++i) {
        uint32_t glyph = glyphs[i];
        if (i == cursorPosition && highlightCurrent) {
            glyph = ~glyph;
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
                dumpByte(glyph, 16, 3);
                dumpByte(glyph, 8, 8);
                dumpByte(glyph, 0, 8);
                Serial.println();
                return;
            }
            case 4:
                glyphs[cursorPosition] = 0;
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
        bitToggle(glyphs[cursorPosition], bit);
        return;
    }
    Serial.print(F("Unknown command: "));
    Serial.println(command);
}
