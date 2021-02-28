#pragma once

class CriticalSection {

    const uint8_t sreg;

public:

    CriticalSection();
    ~CriticalSection();
};
