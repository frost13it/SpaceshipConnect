#include <Arduino.h>

#include "CriticalSection.h"

CriticalSection::CriticalSection() : sreg(SREG) {
    cli();
}

CriticalSection::~CriticalSection() {
    SREG = sreg;
}
