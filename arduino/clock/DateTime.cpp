#include "DateTime.h"
#include "string.h"

static const char DAYS_OF_WEEK_SHORT_ENG[] PROGMEM = "MoTuWeThFrSaSu";
static const char DAYS_OF_WEEK_SHORT_RUS[] PROGMEM = "пнвтсрчтптсбвс";
static const char DAYS_OF_WEEK_MEDIUM_ENG[] PROGMEM = "MonTueWedThuFriSatSun";
static const char DAYS_OF_WEEK_MEDIUM_RUS[] PROGMEM = " пн вт ср чт пт сб вс";
static const char MONTHS_ENG[] PROGMEM = "JanFebMarAprMayJunJulAugSepOctNovDec";
static const char MONTHS_RUS[] PROGMEM = "янвфевмарапрмаяиюниюлавгсеноктноядек";
static const char INVALID_DATE_STRING[] PROGMEM = "error";

typedef Date::Locale Locale;

const Locale Locale::ENG = {
        DAYS_OF_WEEK_SHORT_ENG,
        DAYS_OF_WEEK_MEDIUM_ENG,
        MONTHS_ENG,
};
const Locale Locale::RUS = {
        DAYS_OF_WEEK_SHORT_RUS,
        DAYS_OF_WEEK_MEDIUM_RUS,
        MONTHS_RUS,
};

static char *appendNumber(char *dest, uint16_t number, uint8_t size, char padding) {
    if (number > 0) {
        auto ptr = dest + size - 1;
        while (ptr >= dest) {
            auto digit = '0' + (number % 10);
            *ptr-- = number ? (char) digit : padding;
            number /= 10;
        }
    } else {
        memset(dest, ' ', size);
    }
    return dest + size;
}

static char *appendName(char *dest, PGM_P names, int8_t index, uint8_t len) {
    if (index >= 0) {
        memcpy_P(dest, names + len * index, len);
    } else {
        memset(dest, ' ', len);
    }
    return dest + len;
}

char *Date::format(char *dest, const char *format, const Locale &locale) const {
    if (!isValid()) {
        strcpy_P(dest, INVALID_DATE_STRING);
        return dest + sizeof(INVALID_DATE_STRING) - 1;
    }

    while (char nextChar = *(format++)) {
        switch (nextChar) {
            case 'w':
                dest = appendName(dest, locale.daysOfWeekShort, dayOfWeek - 1, Locale::DAY_OF_WEEK_SHORT_LENGTH);
                break;
            case 'W':
                dest = appendName(dest, locale.daysOfWeekMedium, dayOfWeek - 1, Locale::DAY_OF_WEEK_MEDIUM_LENGTH);
                break;
            case 'd':
                dest = appendNumber(dest, dayOfMonth, 2, ' ');
                break;
            case 'D':
                dest = appendNumber(dest, dayOfMonth, 2, '0');
                break;
            case 'm':
                dest = appendNumber(dest, month, 2, '0');
                break;
            case 'M':
                dest = appendName(dest, locale.months, month - 1, Locale::MONTH_LENGTH);
                break;
            case 'Y':
                dest = appendNumber(dest, 2000 + year, 4, '0');
                break;
            case 'y':
                dest = appendNumber(dest, year, 2, '0');
                break;
            default:
                *dest++ = nextChar;
        }
    }
    *dest = '\0';
    return dest;
}

bool Date::isValid() const {
    return year <= 99 &&
           month > 0 && month <= 12 &&
           dayOfMonth > 0 && dayOfMonth <= 31 &&
           dayOfWeek > 0 && dayOfWeek <= 7;
}

bool Date::operator==(const Date &other) const {
    return !memcmp(this, &other, sizeof(Date));
}

bool Date::operator!=(const Date &other) const {
    return !(*this == other);
}

bool Time::isValid() const {
    return hour < 24 && minute < 60 && second < 60;
}

bool DateTime::isValid() const {
    return time.isValid() && date.isValid();
}

uint32_t Time::totalSeconds() const {
    return second + (minute + hour * 60) * 60ul;
}
