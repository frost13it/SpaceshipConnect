#pragma once

#include <Print.h>

class TimedSection {

    Print &out;
    const __FlashStringHelper *name;
    const unsigned long startTime;

public:

    explicit TimedSection(Print &out, const __FlashStringHelper *name);

    ~TimedSection();

};


