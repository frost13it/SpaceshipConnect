#include "../../display/SpaceshipDisplay.h"
#include "../../environment.h"

using namespace msm6775;

SegmentDriver segmentDriver(DISPLAY_CLOCK, DISPLAY_DATA, DISPLAY_CE);
SegmentsState segments;
SpaceshipDisplay display(segments);

void setup() {
    Serial.begin(SERIAL_SPEED);
    display.commitState(segmentDriver);
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
        segments.writeSegment(segmentIndex, true);
    }
    display.commitState(segmentDriver);

    char command = serialRead();
    switch (command) {
        case 'a':
            segmentIndex = (segmentIndex + SegmentsState::SEGMENTS_COUNT - 1) % SegmentsState::SEGMENTS_COUNT;
            break;
        case 'd':
            segmentIndex = (segmentIndex + 1) % SegmentsState::SEGMENTS_COUNT;
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
