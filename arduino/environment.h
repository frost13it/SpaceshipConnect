#pragma once

#define SERIAL_SPEED 115200

// the pin must support external interrupts
#define DISPLAY_EMULATOR_CE 2

#define DISPLAY_CLOCK 3
#define DISPLAY_DATA 4
#define DISPLAY_CE 5

#define HVAC_CLOCK 7
#define HVAC_DATA 8

#define RTC_CLOCK A1
#define RTC_DATA A2
#define RTC_CE A3

#define ONE_WIRE_PIN 6
#define TEMP_SENSOR_ADDRESS {0x28, 0x3E, 0x2C, 0x3A, 0x26, 0x20, 0x01, 0xB0}

#define REVERSING_PIN A0

#define AUDIO_SW_PIN 9
#define HFT_SW_PIN 10
