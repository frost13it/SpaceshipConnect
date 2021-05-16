#include "Settings.h"

const char *DATE_FORMATS[] = {
        //                     ___________
        "w D\x95m\x95y", // 0: Tu 01•09•20
        "W d M",         // 1: Tue  1 Sep
        "D-m-Y"          // 2: 01-09-2020
};

const Settings Settings::DEFAULT_SETTINGS = {
        dateLocale: &Date::Locale::RUS,
        dateFormat: DATE_FORMATS[1],
        dateCaps: true,
};

bool SavedSettings::load(Settings &outSettings) const {
    switch (dateLocaleIndex) {
        case 0:
            outSettings.dateLocale = &Date::Locale::ENG;
            break;
        case 1:
            outSettings.dateLocale = &Date::Locale::RUS;
            break;
        default:
            return false;
    }
    if (dateFormatIndex < (sizeof DATE_FORMATS / sizeof *DATE_FORMATS)) {
        outSettings.dateFormat = DATE_FORMATS[dateFormatIndex];
    } else {
        return false;
    }
    outSettings.dateCaps = dateCaps;
    return true;
}
