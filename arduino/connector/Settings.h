#pragma once

#include "../clock/DateTime.h"

struct Settings {

    const Date::Locale *dateLocale;
    const char *dateFormat;
    bool dateCaps;

    static const Settings DEFAULT_SETTINGS;

};

struct SavedSettings {
    uint8_t dateLocaleIndex;
    uint8_t dateFormatIndex;
    uint8_t dateCaps;

    bool load(Settings &outSettings) const;
};
