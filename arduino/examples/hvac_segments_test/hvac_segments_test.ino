#include <Arduino.h>
#include "../../display/SpaceshipDisplay.h"
#include "../../environment.h"

msm6775::SegmentDisplay segmentDisplay(DISPLAY_CLOCK, DISPLAY_DATA, DISPLAY_CE);
SpaceshipDisplay display(segmentDisplay);

bool modeFront = false;
bool modeBottom = false;
bool modeWindshield = false;
int8_t leftTemp = SpaceshipDisplay::Hvac::TEMP_LO;
int8_t rightTemp = SpaceshipDisplay::Hvac::TEMP_HI;
uint8_t acMode = 0;
int8_t fanSpeed = -1;

void setup() {
    Serial.begin(SERIAL_SPEED);

    display.text.showString(0, "Honda Civic");
    for (int frame = 0; frame < 15; ++frame) {
        auto spinnerFrame = frame % SpaceshipDisplay::NUM_SPINNER_FRAMES;
        int8_t number = SpaceshipDisplay::NUM_SPINNER + spinnerFrame;
        display.clock.minute(number);
        display.hvac.leftTemp(number, false);
        display.hvac.rightTemp(number, false);
        display.update();
        delay(200);
    }

    display.clearAll();
    display.update();
}

void loop() {
    auto hvac = display.hvac;
    hvac.mode(modeWindshield, modeFront || modeBottom, modeFront, modeBottom);
    hvac.ac(acMode == 1, acMode == 2);
    hvac.fan(fanSpeed >= 0, max(fanSpeed, 0));
    hvac.leftAuto(leftTemp == 20);
    hvac.leftTemp(leftTemp, leftTemp <= 40);
    hvac.rightAuto(rightTemp == 20);
    hvac.rightFull(rightTemp == 30);
    hvac.rightTemp(rightTemp, rightTemp <= 40);
    display.update();

    int command = Serial.read();
    if (command == -1) return;
    switch (command) {
        case 'q':
            modeWindshield = !modeWindshield;
            return;
        case 'w':
            modeFront = !modeFront;
            return;
        case 'e':
            modeBottom = !modeBottom;
            return;
        case 'a':
            acMode = (acMode + 1) % 3;
            return;
        case 's':
            fanSpeed = max(fanSpeed - 1, -1);
            return;
        case 'd':
            fanSpeed = min(fanSpeed + 1, SpaceshipDisplay::Hvac::MAX_FAN_SPEED);
            return;
        case 'z':
            leftTemp--;
            return;
        case 'x':
            leftTemp++;
            return;
        case 'c':
            rightTemp--;
            return;
        case 'v':
            rightTemp++;
            return;
    }
    Serial.print(F("Unknown command: "));
    Serial.println(command);
}
