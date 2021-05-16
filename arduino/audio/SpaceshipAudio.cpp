#include <string.h>

#include "SpaceshipAudio.h"

typedef SpaceshipDisplay::Text Text;
typedef SpaceshipDisplay::Glyph Glyph;

static const Glyph AUX_GLYPHS[] = {
        Text::getFontGlyph('A'),
        Text::getFontGlyph('U'),
        Text::getFontGlyph('X'),
};

static bool isAllBlank(const Glyph *glyphs, uint8_t startOffset) {
    auto glyphsData = reinterpret_cast<const uint8_t *>(glyphs);
    for (uint8_t i = startOffset; i < Text::SIZE * sizeof(Glyph); ++i) {
        if (glyphsData[i]) return false;
    }
    return true;
}

static bool isAuxGlyphs(const Glyph *glyphs) {
    if (memcmp(glyphs, AUX_GLYPHS, sizeof AUX_GLYPHS)) return false;
    return isAllBlank(glyphs, sizeof AUX_GLYPHS);
}

void SpaceshipAudio::refreshState() {
    SpaceshipDisplay::Glyph glyphs[Text::SIZE];
    display.text.getGlyphs(glyphs);
    _isSwitchedOn = !isAllBlank(glyphs, 0);
    _isAuxDisplayed = isAuxGlyphs(glyphs);
}

bool SpaceshipAudio::isSwitchedOn() {
    return _isSwitchedOn;
}

bool SpaceshipAudio::isAuxModeDisplayed() {
    return _isAuxDisplayed;
}

void SpaceshipAudio::onSpiTransferFinished() {
    displayEmulator.onSpiTransferFinished();
}
