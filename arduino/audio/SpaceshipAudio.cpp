#include "SpaceshipAudio.h"

void SpaceshipAudio::onSpiTransferFinished() {
    displayEmulator.onSpiTransferFinished();
}

bool SpaceshipAudio::isSwitchedOn() {
    return _isSwitchedOn;
}

void SpaceshipAudio::refreshState() {
    SpaceshipDisplay::Glyph glyphs[SpaceshipDisplay::Text::SIZE];
    display.text.getGlyphs(glyphs);
    uint8_t *glyphsData = reinterpret_cast<uint8_t *>(glyphs);
    bool hasNonBlankGlyph = false;
    for (uint8_t i = 0; i < sizeof glyphs; ++i) {
        if (glyphsData[i]) {
            hasNonBlankGlyph = true;
            break;
        }
    }
    _isSwitchedOn = hasNonBlankGlyph;
}
