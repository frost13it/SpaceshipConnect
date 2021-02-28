#include <Arduino.h>

#include "../../clock/ds1302.h"
#include "../../environment.h"

Ds1302 rtc(RTC_CLOCK, RTC_DATA, RTC_CE);
DateTime currentDate;

void setup() {
    Serial.begin(SERIAL_SPEED);
    while (!Serial);

    Serial.print("Clock halt: ");
    Serial.println(rtc.isClockHalt());
    Serial.print("Write protect: ");
    Serial.println(rtc.isWriteProtect());
    DateTime last;
    rtc.readRam(&last, sizeof(DateTime));
    Serial.print("Last saved: ");
    printDateTime(last);

    if (rtc.getDateTime().date.year < 21) {
        DateTime dt = {{2, 5, 1, 21}, {0, 58, 0}};
        rtc.setDateTime(dt, false, false);
    }
}

void loop() {
    auto dateTime = rtc.getDateTime();
    if (memcmp(&dateTime, &currentDate, sizeof(DateTime))) {
        printDateTime(dateTime);
        currentDate = dateTime;
        rtc.writeRam(&currentDate, sizeof(DateTime));
    }
    delay(100);
}

void printDateTime(DateTime &dt) {
    auto &s = Serial;
    s.print(dt.time.hour);
    s.print(":");
    s.print(dt.time.minute);
    s.print(":");
    s.print(dt.time.second);
    s.print(" ");
    char dateStr[20];
    dt.date.format(dateStr, "W d M Y", Date::Locale::ENG);
    s.println(dateStr);
}
