#include <Arduino.h>
#include "TimedSection.h"

TimedSection::TimedSection(Print &out, const __FlashStringHelper *name) : out(out), name(name), startTime(micros()) {}

TimedSection::~TimedSection() {
    auto totalTime = micros() - startTime;
    out.print(name);
    out.print(F(" performed in "));
    out.print(totalTime);
    out.println(" us");
}
