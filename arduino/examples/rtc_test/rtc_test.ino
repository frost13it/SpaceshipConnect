#include <Arduino.h>
#include "../../clock/ds1302.h"
#include "../../environment.h"

Ds1302 rtc(RTC_CLOCK, RTC_DATA, RTC_CE);
DateTime lastDt;
int32_t origin;
int32_t lastDiff = 0;

void setup() {
    Serial.begin(SERIAL_SPEED);
    while (!Serial);
    dumpState();
    auto now = micros();
    DateTime dt = rtc.getDateTime();
    origin = (int32_t) now - (int32_t) dt.time.totalSeconds() * 1000000;
    if (!dt.isValid()) {
        dt.date.year = 21;
        dt.date.month = 6;
        dt.date.dayOfMonth = 16;
        dt.date.dayOfWeek = 3;
        dt.time.hour = 0;
        dt.time.minute = 0;
        dt.time.second = 0;
        rtc.setWriteProtect(false);
        rtc.setDateTime(dt, false, true);
        dumpState();
    }
}

void loop() {
    auto now = micros();
    auto dt = rtc.getDateTime();
    if (!memcmp(&dt, &lastDt, sizeof dt)) {
        return;
    }
    lastDt = dt;
    printDateTime(dt, now);
    lastDiff = (int32_t) dt.time.totalSeconds() * 1000000 - (int32_t) now + origin;
}

static void dumpState() {
    Serial.print("CH");
    Serial.print(rtc.isClockHalt() ? '+' : '-');
    Serial.print(" ");
    Serial.print("WP");
    Serial.print(rtc.isWriteProtect() ? '+' : '-');
    Serial.print(" ");
    Serial.print("TC: ");
    Serial.print(rtc.getTrickleChargingConfig());
    Serial.print(", ");
    auto dt = rtc.getDateTime();
    auto now = micros();
    printDateTime(dt, now);
    Serial.print("RAM: ");
    uint8_t ram[31];
    rtc.readRam(ram, sizeof ram);
    for (uint8_t byte : ram) {
        Serial.print(byte >> 4, HEX);
        Serial.print(byte & 0xf, HEX);
    }
    Serial.println();
}

static void printDateTime(DateTime &dt, uint32_t now) {
    char dateStr[20];
    dt.date.format(dateStr, "W d M Y", Date::Locale::ENG);
    Serial.print(dateStr);
    Serial.print(" ");
    Serial.print(dt.time.hour);
    Serial.print(":");
    Serial.print(dt.time.minute);
    Serial.print(":");
    Serial.print(dt.time.second);
    Serial.print(" / ");
    auto diff = (int32_t) dt.time.totalSeconds() * 1000000 - (int32_t) now + origin;
    Serial.print(diff);
    Serial.print(" / ");
    Serial.print(diff - lastDiff);
    Serial.println();
}
