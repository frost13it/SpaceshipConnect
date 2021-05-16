#pragma once

#include "msm6775.h"
#include "WString.h"

class SpaceshipDisplay {

    static const uint8_t DIGITS[] PROGMEM;
    static constexpr uint8_t DIGIT_BLANK = 10;
    static constexpr uint8_t DIGIT_DASH = 11;
    static constexpr uint8_t DIGIT_ERROR = 12;
    static constexpr uint8_t DIGIT_H = 13;
    static constexpr uint8_t DIGIT_I = 14;
    static constexpr uint8_t DIGIT_L = 15;
    static constexpr uint8_t DIGIT_O = 16;
    static constexpr uint8_t DIGIT_SPINNER = 17; // + 18, 19

    static void renderTwoDigitNumber(uint8_t *digits, int8_t value);

public:

    static constexpr int8_t NUM_BLANK = -101;
    static constexpr int8_t NUM_DASH = -102;
    static constexpr int8_t NUM_SPINNER = -105;
    static constexpr uint8_t NUM_SPINNER_FRAMES = 3;

    msm6775::SegmentsState segments;

    void commitState(msm6775::SegmentDriver &driver);

    void clearAll();

    void setAll();

    struct Glyph {
        uint8_t data[3];

        uint32_t toUint32() const;

        static const Glyph from(uint32_t data);

        static const Glyph BLANK;

    };

    class Section {

    protected:

        msm6775::SegmentsState &segments;

        explicit Section(msm6775::SegmentsState &segments) : segments(segments) {};

    public:

        virtual void clear() = 0;

    };

    // Clock section
    class Clock : Section {

        friend class SpaceshipDisplay;

        explicit Clock(msm6775::SegmentsState &segments) : Section(segments) {}

        void writeFirstDigit(uint8_t value);

        void writeFullDigit(uint8_t offset, uint8_t value);

    public:

        void clear() override;

        void hour(int8_t value);

        void colon(bool value);

        void minute(int8_t value);
    };

    // Text section
    class Text : Section {

        static const uint8_t GLYPHS[] PROGMEM;

        friend class SpaceshipDisplay;

        explicit Text(msm6775::SegmentsState &segments) : Section(segments) {}

    public:

        static constexpr uint8_t SIZE = 11;
        static constexpr uint32_t SUBSTITUTE_GLYPH_DATA = 0b0000000001000000000;
        static const Glyph SUBSTITUTE_GLYPH;

        static Glyph getFontGlyph(char character);

        void clear() override;

        void showString(uint8_t startIndex, const char *string);

        void showString(uint8_t startIndex, const __FlashStringHelper *string);

        void showCharacter(uint8_t index, char character);

        void showGlyph(uint8_t index, const Glyph glyph);

        Glyph getGlyph(uint8_t index) const;

        void getGlyphs(Glyph *dest) const;

        void dash(bool value);

        void backtick(bool value);

        void dot(bool value);

    };

    // Climate section
    class Hvac : Section {

        friend class SpaceshipDisplay;

        explicit Hvac(msm6775::SegmentsState &segments) : Section(segments) {}

        void writeTemp(uint8_t offset, bool frame, int8_t value);

        void writeTempDigit(uint8_t offset, uint8_t value);

    public:

        static constexpr int8_t TEMP_HI = -106;
        static constexpr int8_t TEMP_LO = -107;

        static constexpr uint8_t MAX_FAN_SPEED = 7;

        void clear() override;

        void leftTemp(int8_t value);

        void leftTemp(int8_t value, bool frame);

        void leftAuto(bool value);

        void rightTemp(int8_t value);

        void rightTemp(int8_t value, bool frame);

        void rightAuto(bool value);

        void rightFull(bool value);

        void fan(bool icon, uint8_t speed);

        void ac(bool on, bool off);

        void mode(bool windshield, bool human, bool front, bool bottom);

    };

    // Media indicators
    class Media : Section {

        friend class SpaceshipDisplay;

        explicit Media(msm6775::SegmentsState &segments) : Section(segments) {}

    public:

        void clear() override;

        void wma(bool value);

        void mp3(bool value);

        void asel(bool value);

        void ta(bool value);

        void tp(bool value);

        void tel(bool value);

        void wireless(bool value);

        void track(bool value);

        void folder(bool value);

        void disk(bool value);

        void dolby(bool value);

        void channel(bool value);

        void news(bool value);

        void st(bool value);

        void scan(bool d, bool f, bool scan);

        void rpt(bool d, bool f, bool rpt);

        void rdm(bool f, bool rdm);

        void diskNumber(uint8_t number, bool value);

    };

    Clock clock;
    Text text;
    Hvac hvac;
    Media media;

    explicit SpaceshipDisplay() :
            clock(segments),
            text(segments),
            hvac(segments),
            media(segments) {}

};
