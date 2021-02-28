#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>

struct Date {

    struct Locale;

    // First is 1
    uint8_t dayOfWeek;
    // First is 1
    uint8_t dayOfMonth;
    // First is 1
    uint8_t month;
    // First is 2000
    uint8_t year;

    /*
     * Format patterns:
     * - w: day of week, 2 chars
     * - W: day of week, 3 chars
     * - d: day of month, 2 chars space-padded
     * - D: day of month, 2 chars zero-padded
     * - m: month (number), 2 chars zero-padded
     * - M: month (name), 3 symbols
     * - y: year since 2000, 2 chars zero-padded
     * - Y: year (20xx), 4 chars
     *
     * Returns pointer to the end (\0)
     */
    char *format(char *dest, const char *format, const Locale &locale) const;

    bool isValid() const;

    bool operator==(const Date &other) const;

    bool operator!=(const Date &other) const;

    struct Locale {
        PGM_P daysOfWeekShort;
        PGM_P daysOfWeekMedium;
        PGM_P months;

        static const uint8_t DAY_OF_WEEK_SHORT_LENGTH = 2;
        static const uint8_t DAY_OF_WEEK_MEDIUM_LENGTH = 3;
        static const uint8_t MONTH_LENGTH = 3;

        static const Locale RUS;
        static const Locale ENG;
    };

};

struct Time {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    uint32_t totalSeconds() const;

    bool isValid() const;

};

struct DateTime {
    Date date;
    Time time;

    bool isValid() const;

};

