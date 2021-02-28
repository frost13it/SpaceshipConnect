#include "../../display/SpaceshipDisplay.h"
#include "../../environment.h"

using namespace msm6775;

SegmentDisplay segmentDisplay(DISPLAY_CLOCK, DISPLAY_DATA, DISPLAY_CE);
SpaceshipDisplay display(segmentDisplay);

void setup() {
    Serial.begin(SERIAL_SPEED);
    display.update();
}

uint8_t serialRead() {
    int result;
    while((result = Serial.read()) == -1);
    return result;
}

int segmentIndex = 0;
bool allSet = false;
bool showNumber = true;

void loop() {
    if (allSet) {
        display.setAll();
    } else {
        display.clearAll();
        if (showNumber) {
            display.clock.hour(segmentIndex / 100);
            display.clock.minute(segmentIndex % 100);
        }
        segmentDisplay.writeSegment(segmentIndex, true);
    }
    display.update();

    char command = serialRead();
    switch (command) {
        case 'a':
            segmentIndex = (segmentIndex + SegmentDisplay::SEGMENTS_COUNT - 1) % SegmentDisplay::SEGMENTS_COUNT;
            break;
        case 'd':
            segmentIndex = (segmentIndex + 1) % SegmentDisplay::SEGMENTS_COUNT;
            break;
        case 's':
            allSet = !allSet;
            break;
        case 'w':
            showNumber = !showNumber;
            break;
        default:
            Serial.print(F("Unknown command: "));
            Serial.println(command);
            break;
    }
}
