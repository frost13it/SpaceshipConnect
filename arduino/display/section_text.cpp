#include "SpaceshipDisplay.h"

typedef SpaceshipDisplay::Text Text;
typedef SpaceshipDisplay::Glyph Glyph;

static constexpr uint8_t FIRST_GLYPH_CODE = 0x20;
static constexpr uint8_t S_G_1 = Text::SUBSTITUTE_GLYPH_DATA >> 16;
static constexpr uint8_t SUBST_GL_2 = Text::SUBSTITUTE_GLYPH_DATA >> 8;
static constexpr uint8_t SUBST_GL_3 = Text::SUBSTITUTE_GLYPH_DATA & 0xFF;

/*
 * Font for 19-segment text sections. Text encoding is codepage 1251.
 * Numeration of bits:
 *              0
 *     1                 5
 *         2    3    4
 *     6                 7
 *         8    9   10
 *    11                 12
 *        13    14  15
 *    16                 17
 *              18
 */
const uint8_t Text::GLYPHS[] = {
        0b000, 0b00000000, 0b00000000, // 20, space
        0b001, 0b00000000, 0b01000010, // 21, !
        0b000, 0b00000000, 0b00010100, // 22, "
        0b000, 0b11100101, 0b00011100, // 23, #
        0b111, 0b01010111, 0b01101011, // 24, $
        0b011, 0b00100010, 0b00110010, // 25, %
        0b111, 0b10011011, 0b00010111, // 26, &
        0b000, 0b00000000, 0b00001000, // 27, '
        0b010, 0b10000100, 0b00110000, // 28, (
        0b001, 0b00100001, 0b00000110, // 29, )
        0b000, 0b11100010, 0b00011100, // 2a, *
        0b000, 0b01000111, 0b00001000, // 2b, +
        0b001, 0b00100000, 0b00000000, // 2c, ,
        0b000, 0b00000111, 0b00000000, // 2d, -
        0b001, 0b00000000, 0b00000000, // 2e, .
        0b001, 0b00100010, 0b00110000, // 2f, /
        0b111, 0b00111010, 0b11110011, // 30, 0
        0b010, 0b00010000, 0b10100000, // 31, 1
        0b101, 0b00001111, 0b10100001, // 32, 2
        0b110, 0b00010111, 0b10100001, // 33, 3
        0b010, 0b00010111, 0b11100010, // 34, 4
        0b110, 0b00010111, 0b01000011, // 35, 5
        0b111, 0b00011111, 0b01000011, // 36, 6
        0b010, 0b00010000, 0b10100001, // 37, 7
        0b111, 0b00011111, 0b11100011, // 38, 8
        0b110, 0b00010111, 0b11100011, // 39, 9
        0b000, 0b01000000, 0b00001000, // 3a, :
        0b001, 0b00100000, 0b00000100, // 3b, ;
        0b010, 0b10000010, 0b00110000, // 3c, <
        0b100, 0b00000111, 0b00000000, // 3d, =
        0b001, 0b00100010, 0b00000110, // 3e, >
        0b001, 0b00000111, 0b10100011, // 3f, ?
        0b111, 0b00001110, 0b11101011, // 40, @
        0b011, 0b00011111, 0b11100011, // 41, A
        0b110, 0b01010110, 0b10101001, // 42, B
        0b101, 0b00001000, 0b01000011, // 43, C
        0b110, 0b01010010, 0b10101001, // 44, D
        0b101, 0b00001111, 0b01000011, // 45, E
        0b001, 0b00001111, 0b01000011, // 46, F
        0b111, 0b00011100, 0b01000011, // 47, G
        0b011, 0b00011111, 0b11100010, // 48, H
        0b100, 0b01000010, 0b00001001, // 49, I
        0b111, 0b00010000, 0b10100000, // 4a, J
        0b011, 0b10001011, 0b01110010, // 4b, K
        0b101, 0b00001000, 0b01000010, // 4c, L
        0b011, 0b00011010, 0b11110110, // 4d, M
        0b011, 0b10011010, 0b11100110, // 4e, N
        0b111, 0b00011000, 0b11100011, // 4f, O
        0b001, 0b00001111, 0b11100011, // 50, P
        0b111, 0b10011000, 0b11100011, // 51, Q
        0b011, 0b10001111, 0b11100011, // 52, R
        0b111, 0b00010111, 0b01100011, // 53, S
        0b000, 0b01000010, 0b00001001, // 54, T
        0b111, 0b00011000, 0b11100010, // 55, U
        0b001, 0b00101010, 0b01110010, // 56, V
        0b011, 0b10111010, 0b11100010, // 57, W
        0b011, 0b10100010, 0b00110110, // 58, X
        0b000, 0b01000010, 0b00110110, // 59, Y
        0b111, 0b00100010, 0b00110011, // 5a, Z
        0b101, 0b00001000, 0b01000011, // 5b, [
        0b010, 0b10000010, 0b00000110, // 5c, (\)
        0b110, 0b00010000, 0b10100001, // 5d, ]
        0b000, 0b00011101, 0b00001000, // 5e, ^
        0b100, 0b00000000, 0b00000000, // 5f, _
        0b000, 0b00000000, 0b00000110, // 60, `
        0b101, 0b01001011, 0b00000000, // 61, a
        0b111, 0b00011111, 0b01000010, // 62, b
        0b101, 0b00001111, 0b00000000, // 63, c
        0b111, 0b00011111, 0b10100000, // 64, d
        0b101, 0b00001111, 0b11100011, // 65, e
        0b001, 0b00001011, 0b01000011, // 66, f
        0b111, 0b00010111, 0b11100011, // 67, g
        0b011, 0b00011111, 0b01000010, // 68, h
        0b001, 0b00001000, 0b00000010, // 69, i
        0b110, 0b00010000, 0b00100000, // 6a, j
        0b011, 0b10101010, 0b01010010, // 6b, k
        0b101, 0b00100010, 0b00001000, // 6c, l
        0b011, 0b01011111, 0b00000000, // 6d, m
        0b010, 0b01010110, 0b00000000, // 6e, n
        0b111, 0b00011111, 0b00000000, // 6f, o
        0b001, 0b00001011, 0b01010011, // 70, p
        0b010, 0b00010110, 0b10100101, // 71, q
        0b001, 0b00001011, 0b00000000, // 72, r
        0b111, 0b10000010, 0b00100111, // 73, s
        0b101, 0b00001011, 0b01000010, // 74, t
        0b101, 0b01001010, 0b01000000, // 75, u
        0b001, 0b00101010, 0b01000000, // 76, v
        0b111, 0b01011010, 0b11000000, // 77, w
        0b000, 0b10100010, 0b00010100, // 78, x
        0b110, 0b00010111, 0b11100010, // 79, y
        0b101, 0b00100111, 0b00110001, // 7a, z
        0b010, 0b10000110, 0b00110000, // 7b, {
        0b000, 0b01000010, 0b00001000, // 7c, |
        0b001, 0b00100011, 0b00000110, // 7d, }
        0b000, 0b00001111, 0b10000000, // 7e, ~
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 7f, DEL (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 80, Ђ (unsupported)
        0b001, 0b00001000, 0b01000011, // 81, Ѓ
        0b000, 0b01000000, 0b00000000, // 82, ‚
        0b001, 0b00001011, 0b00000010, // 83, ѓ
        0b000, 0b10100000, 0b00000000, // 84, „
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 85, … (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 86, † (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 87, ‡ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 88, € (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 89, ‰ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 8a, Љ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 8b, ‹ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 8c, Њ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 8d, Ќ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 8e, Ћ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 8f, Џ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 90, ђ (unsupported)
        0b000, 0b00000000, 0b00010000, // 91, ‘
        0b000, 0b00000000, 0b00010000, // 92, ’
        0b000, 0b00000000, 0b00010100, // 93, “
        0b000, 0b00000000, 0b00010100, // 94, ”
        0b000, 0b00000010, 0b00000000, // 95, •
        0b000, 0b00000111, 0b00000000, // 96, –
        0b000, 0b00000111, 0b00000000, // 97, —
        0b111, 0b10111010, 0b11110111, // 98 (unmapped)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 99, ™ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 9a, љ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 9b, › (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 9c, њ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 9d, ќ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 9e, ћ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // 9f, џ (unsupported)
        0b000, 0b00000000, 0b00000000, // a0, nbsp
        S_G_1, SUBST_GL_2, SUBST_GL_3, // a1, Ў (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // a2, ў (unsupported)
        0b111, 0b00010000, 0b10100000, // a3, Ј
        S_G_1, SUBST_GL_2, SUBST_GL_3, // a4, ¤ (unsupported)
        0b001, 0b00001000, 0b01000011, // a5, Ґ
        0b000, 0b01000000, 0b00001000, // a6, ¦
        S_G_1, SUBST_GL_2, SUBST_GL_3, // a7, § (unsupported)
        0b101, 0b00001111, 0b01000011, // a8, Ё
        S_G_1, SUBST_GL_2, SUBST_GL_3, // a9, © (unsupported)
        0b111, 0b00001011, 0b01100011, // aa, Є
        S_G_1, SUBST_GL_2, SUBST_GL_3, // ab, « (unsupported)
        0b000, 0b00010111, 0b00000000, // ac, ¬
        S_G_1, SUBST_GL_2, SUBST_GL_3, // ad, SHY (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // ae, ® (unsupported)
        0b100, 0b01000111, 0b00100010, // af, Ї
        0b000, 0b00000111, 0b11100011, // b0, °
        0b100, 0b01000111, 0b00001000, // b1, ±
        0b100, 0b01000010, 0b00001001, // b2, І
        0b001, 0b00001000, 0b00000010, // b3, і
        S_G_1, SUBST_GL_2, SUBST_GL_3, // b4, ґ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // b5, µ (unsupported)
        S_G_1, SUBST_GL_2, SUBST_GL_3, // b6, ¶ (unsupported)
        0b000, 0b00000010, 0b00000000, // b7, ·
        0b101, 0b00001111, 0b11100011, // b8, ё
        S_G_1, SUBST_GL_2, SUBST_GL_3, // b9, № (unsupported)
        0b111, 0b00001011, 0b01100011, // ba, є
        S_G_1, SUBST_GL_2, SUBST_GL_3, // bb, » (unsupported)
        0b110, 0b00010000, 0b00100000, // bc, ј
        0b111, 0b00010111, 0b01100011, // bd, Ѕ
        0b111, 0b10000010, 0b00100111, // be, s
        0b000, 0b01000000, 0b00010100, // bf, ї
        0b011, 0b00011111, 0b11100011, // c0, А
        0b111, 0b00011111, 0b01000011, // c1, Б
        0b111, 0b10001011, 0b01110011, // c2, В
        0b001, 0b00001000, 0b01000011, // c3, Г
        0b110, 0b01010010, 0b10101001, // c4, Д
        0b101, 0b00001111, 0b01000011, // c5, Е
        0b011, 0b11100111, 0b00111110, // c6, Ж
        0b111, 0b10000010, 0b00110011, // c7, З
        0b011, 0b00111010, 0b11110010, // c8, И
        0b011, 0b00111010, 0b11110010, // c9, Й
        0b011, 0b10001011, 0b01110010, // ca, К
        0b011, 0b00110010, 0b10101001, // cb, Л
        0b011, 0b00011010, 0b11110110, // cc, М
        0b011, 0b00011111, 0b11100010, // cd, Н
        0b111, 0b00011000, 0b11100011, // ce, О
        0b011, 0b00011000, 0b11100011, // cf, П
        0b001, 0b00001111, 0b11100011, // d0, Р
        0b111, 0b00001000, 0b01100011, // d1, С
        0b000, 0b01000010, 0b00001001, // d2, Т
        0b001, 0b00100010, 0b00110110, // d3, У
        0b000, 0b01000111, 0b11101011, // d4, Ф
        0b011, 0b10100010, 0b00110110, // d5, Х
        0b101, 0b01001010, 0b01001010, // d6, Ц
        0b010, 0b00010111, 0b11100010, // d7, Ч
        0b111, 0b01011010, 0b11101010, // d8, Ш
        0b111, 0b01011010, 0b11101010, // d9, Щ
        0b111, 0b00011111, 0b01000010, // da, Ъ
        0b101, 0b01011011, 0b11100010, // db, Ы
        0b111, 0b00011111, 0b01000010, // dc, Ь
        0b111, 0b00010111, 0b10100011, // dd, Э
        0b011, 0b10011011, 0b11110010, // de, Ю
        0b011, 0b00110111, 0b11100011, // df, Я
        0b101, 0b01001011, 0b00000000, // e0, а
        0b111, 0b00011111, 0b00000111, // e1, б
        0b111, 0b00011111, 0b01110011, // e2, в
        0b001, 0b00001011, 0b00000000, // e3, г
        0b111, 0b00010111, 0b11100011, // e4, д
        0b101, 0b00001111, 0b11100011, // e5, е
        0b000, 0b11100111, 0b00011100, // e6, ж
        0b111, 0b10000010, 0b00110011, // e7, з
        0b101, 0b01001010, 0b01000000, // e8, и
        0b101, 0b01001010, 0b01000100, // e9, й
        0b011, 0b10101010, 0b01010010, // ea, к
        0b011, 0b00110110, 0b00000000, // eb, л
        0b000, 0b00011010, 0b11110110, // ec, м
        0b001, 0b01001011, 0b01001010, // ed, н
        0b111, 0b00011111, 0b00000000, // ee, о
        0b011, 0b00011111, 0b00000000, // ef, п
        0b001, 0b00001011, 0b01010011, // f0, р
        0b101, 0b00001111, 0b00000000, // f1, с
        0b000, 0b01000111, 0b00000000, // f2, т
        0b000, 0b00100010, 0b00010100, // f3, у
        0b000, 0b01000111, 0b11111110, // f4, ф
        0b000, 0b10100010, 0b00010100, // f5, х
        0b101, 0b01001010, 0b00000000, // f6, ц
        0b010, 0b00010110, 0b10101000, // f7, ч
        0b111, 0b01011000, 0b00000000, // f8, ш
        0b111, 0b01011000, 0b00000000, // f9, щ
        0b111, 0b00011111, 0b01000010, // fa, ъ
        0b101, 0b01011011, 0b11100010, // fb, ы
        0b111, 0b00011111, 0b01000010, // fc, ь
        0b111, 0b00010111, 0b10100011, // fd, э
        0b011, 0b10011011, 0b11110010, // fe, ю
        0b011, 0b00110110, 0b10100111, // ff, я
};

const Glyph Text::SUBSTITUTE_GLYPH = Glyph::from(SUBSTITUTE_GLYPH_DATA);

Glyph Text::getFontGlyph(char character) {
    auto codePoint = (unsigned char) character;
    if (codePoint < FIRST_GLYPH_CODE) return SUBSTITUTE_GLYPH;
    auto glyphPtr = GLYPHS + (codePoint - FIRST_GLYPH_CODE) * 3;
    return Glyph{{
            pgm_read_byte(glyphPtr),
            pgm_read_byte(glyphPtr + 1),
            pgm_read_byte(glyphPtr + 2)
    }};
}

void Text::clear() {
    for (int i = 0; i < SIZE; ++i) {
        showGlyph(i, Glyph::BLANK);
    }
    dash(false);
    backtick(false);
    dot(false);
}

void Text::showString(uint8_t startIndex, const char *string) {
    auto index = startIndex;
    while (index < SIZE) {
        auto nextChar = *string++;
        if (!nextChar) break;
        showCharacter(index++, nextChar);
    }
}

void Text::showString(uint8_t startIndex, const __FlashStringHelper *string) {
    const uint8_t *str = reinterpret_cast<const uint8_t *>(string);
    auto index = startIndex;
    while (index < SIZE) {
        auto nextChar = pgm_read_byte(str++);
        if (!nextChar) break;
        showCharacter(index++, nextChar);
    }
}

void Text::showCharacter(uint8_t index, char character) {
    showGlyph(index, getFontGlyph(character));
}

static uint16_t indexToOffset(uint8_t index) {
    uint16_t offset = index * 5;
    if (index >= 5) offset += 8;
    return offset;
}

void Text::showGlyph(uint8_t index, Glyph glyph) {
    auto offset = indexToOffset(index);
    auto byte2 = glyph.data[2];
    segments.writeSegment(16 + offset, byte2 & 1);
    segments.writeSegment(14 + offset, byte2 & 1 << 1);
    segments.writeSegment(15 + offset, byte2 & 1 << 2);
    segments.writeSegment(96 + offset, byte2 & 1 << 3);
    segments.writeSegment(17 + offset, byte2 & 1 << 4);
    segments.writeSegment(18 + offset, byte2 & 1 << 5);
    segments.writeSegment(94 + offset, byte2 & 1 << 6);
    segments.writeSegment(98 + offset, byte2 & 1 << 7);
    auto byte1 = glyph.data[1];
    segments.writeSegment(95 + offset, byte1 & 1);
    segments.writeSegment(176 + offset, byte1 & 1 << 1);
    segments.writeSegment(97 + offset, byte1 & 1 << 2);
    segments.writeSegment(174 + offset, byte1 & 1 << 3);
    segments.writeSegment(178 + offset, byte1 & 1 << 4);
    segments.writeSegment(175 + offset, byte1 & 1 << 5);
    segments.writeSegment(256 + offset, byte1 & 1 << 6);
    segments.writeSegment(177 + offset, byte1 & 1 << 7);
    auto byte0 = glyph.data[0];
    segments.writeSegment(254 + offset, byte0 & 1);
    segments.writeSegment(258 + offset, byte0 & 1 << 1);
    segments.writeSegment(255 + offset, byte0 & 1 << 2);
}

Glyph Text::getGlyph(uint8_t index) const {
    auto offset = indexToOffset(index);
    uint8_t byte2 = 0;
    if (segments.readSegment(16 + offset)) byte2 |= 1;
    if (segments.readSegment(14 + offset)) byte2 |= 1 << 1;
    if (segments.readSegment(15 + offset)) byte2 |= 1 << 2;
    if (segments.readSegment(96 + offset)) byte2 |= 1 << 3;
    if (segments.readSegment(17 + offset)) byte2 |= 1 << 4;
    if (segments.readSegment(18 + offset)) byte2 |= 1 << 5;
    if (segments.readSegment(94 + offset)) byte2 |= 1 << 6;
    if (segments.readSegment(98 + offset)) byte2 |= 1 << 7;
    uint8_t byte1 = 0;
    if (segments.readSegment(95 + offset)) byte1 |= 1;
    if (segments.readSegment(176 + offset)) byte1 |= 1 << 1;
    if (segments.readSegment(97 + offset)) byte1 |= 1 << 2;
    if (segments.readSegment(174 + offset)) byte1 |= 1 << 3;
    if (segments.readSegment(178 + offset)) byte1 |= 1 << 4;
    if (segments.readSegment(175 + offset)) byte1 |= 1 << 5;
    if (segments.readSegment(256 + offset)) byte1 |= 1 << 6;
    if (segments.readSegment(177 + offset)) byte1 |= 1 << 7;
    uint8_t byte0 = 0;
    if (segments.readSegment(254 + offset)) byte0 |= 1;
    if (segments.readSegment(255 + offset)) byte0 |= 1 << 2;
    if (segments.readSegment(258 + offset)) byte0 |= 1 << 1;
    return Glyph{{byte0, byte1, byte2}};
}

void Text::getGlyphs(Glyph *dest) const {
    for (int i = 0; i < SIZE; ++i) {
        dest[i] = getGlyph(i);
    }
}

void Text::dash(bool value) {
    segments.writeSegment(272, value);
}

void Text::backtick(bool value) {
    segments.writeSegment(277, value);
}

void Text::dot(bool value) {
    segments.writeSegment(305, value);
}
