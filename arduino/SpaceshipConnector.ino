#include <Arduino.h>
#include <HID-Project.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "audio/SpaceshipAudio.h"
#include "buttons/SpaceshipButtons.h"
#include "clock/DateTime.h"
#include "clock/ds1302.h"
#include "connector/Connection.h"
#include "connector/Settings.h"
#include "connector/SpaceshipState.h"
#include "display/SpaceshipDisplay.h"
#include "hvac/SpaceshipHvac.h"
#include "util/CriticalSection.h"
#include "util/TimedSection.h"
#include "environment.h"

// Constant data

const uint16_t LOOP_PERIOD_MS = 50;
const uint32_t LOOP_PERIOD_US = LOOP_PERIOD_MS * 1000L;
const uint8_t SPLASH_DURATION_TICKS = 40;
const uint8_t SPLASH_MIN_BREAK_SEC = 10;
const uint8_t SPLASH_SPINNER_FRAME_PERIOD = 4;
const uint8_t CLOCK_BLINK_PERIOD_TICKS = 20;
const uint16_t TEMP_SENSOR_TIMEOUT_TICKS = 200;

// System state

struct {
    DateTime dateTime;
    SpaceshipState spaceship;
    uint16_t tick = 0;
    uint8_t splashTicks = SPLASH_DURATION_TICKS;
    uint16_t tempSensorUnavailableTicks = TEMP_SENSOR_TIMEOUT_TICKS + 1;
    bool phoneConnected = false;
    uint16_t usbFrameCounter = 0;
    char audioTitle[SpaceshipDisplay::Text::SIZE + 1] = {'\0'};
} state;

// Settings

Settings settings = Settings::DEFAULT_SETTINGS;

struct RtcRamData {
    DateTime lastRun;
    SavedSettings settings;
};

// Peripherals
msm6775::SegmentDriver segmentDriver(DISPLAY_CLOCK, DISPLAY_DATA, DISPLAY_CE);
SpaceshipDisplay display;
SpaceshipAudio audio(DISPLAY_EMULATOR_CE);
SpaceshipHvac hvac(HVAC_CLOCK, HVAC_DATA);
Ds1302 rtc(RTC_CLOCK, RTC_DATA, RTC_CE);
ButtonSet audioButtons = SpaceshipButtons::audioSwitchSlave(AUDIO_SW_PIN);
ButtonSet hftButtons = SpaceshipButtons::hft(HFT_SW_PIN);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature dsTemp(&oneWire);
DeviceAddress tempSensorAddress = TEMP_SENSOR_ADDRESS;
Pin reversingSensor(REVERSING_PIN);

// Connection

Connection connection;
ConnectorRequest request;
ConnectorReply reply;

void setup() {
    display.clock.minute(SpaceshipDisplay::Hvac::TEMP_HI);
    display.text.showString(0, F("Honda Civic"));
    display.commitState(segmentDriver);

    auto now = rtc.getDateTime();
    RtcRamData savedData;
    rtc.readRam(&savedData, sizeof savedData);
    auto &lastRun = savedData.lastRun;
    if (now.isValid() && lastRun.isValid()) {
        auto &savedSettings = savedData.settings;
        Settings loadedSettings;
        if (savedSettings.load(loadedSettings)) {
            settings = loadedSettings;
        }

        if (now.date == lastRun.date && now.time.totalSeconds() - lastRun.time.totalSeconds() <= SPLASH_MIN_BREAK_SEC) {
            // skip splash spinner
            state.splashTicks = 0;
            display.clearAll();
        }
    }

    dsTemp.setWaitForConversion(false);
    dsTemp.setCheckForConversion(false);

    reversingSensor.doInput(true);
    Serial.begin(SERIAL_SPEED);
    Consumer.begin();

    refreshSpaceshipState();
}

void loop() {
    auto startTimeUs = micros();

    const auto dateTime = rtc.getDateTime();
    rtc.writeRam(&dateTime, sizeof dateTime);
    state.dateTime = dateTime;

    SpaceshipState prevState = state.spaceship;
    refreshSpaceshipState();
    updateVirtualKeyboard(prevState);
    if (state.phoneConnected = isUsbConnected()) {
        processInputCommand();
        emitEvents(prevState);
    } else {
        *state.audioTitle = '\0';
    }
    updateDisplay();

    state.usbFrameCounter = getUsbFrameCounter();
    state.tick++;

    static const uint32_t LOOP_TIME_ADJUST_US = 50;
    auto loopTimeUs = micros() - startTimeUs + LOOP_TIME_ADJUST_US;
    if (loopTimeUs < LOOP_PERIOD_US) {
        auto delayTotal = LOOP_PERIOD_US - loopTimeUs;
        uint16_t delayMs = delayTotal / 1000;
        uint16_t delayUs = delayTotal % 1000;
        delay(delayMs);
        delayMicroseconds(delayUs);
    }
}

static bool isUsbConnected() {
    return Serial.dtr() && state.usbFrameCounter != getUsbFrameCounter();
}

static uint16_t getUsbFrameCounter() {
    return UDFNUM;
}

static void refreshSpaceshipState() {
    auto &spaceship = state.spaceship;
    if (!hvac.readState(spaceship.hvac)) {
        spaceship.hvac.connected = false;
    }
    audio.refreshState();
    auto temp = (int8_t) dsTemp.getTempC(tempSensorAddress);
    if (temp != DEVICE_DISCONNECTED_C) {
        auto &currentTemp = spaceship.temp;
        if (currentTemp == SpaceshipDisplay::NUM_BLANK) {
            currentTemp = temp;
        } else {
            if (currentTemp < temp) {
                currentTemp++;
            } else if (currentTemp > temp) {
                currentTemp--;
            }
        }
        state.tempSensorUnavailableTicks = 0;
    } else {
        auto unavailableTime = state.tempSensorUnavailableTicks;
        if (unavailableTime == TEMP_SENSOR_TIMEOUT_TICKS) {
            spaceship.temp = SpaceshipDisplay::NUM_DASH;
        } else if (unavailableTime > TEMP_SENSOR_TIMEOUT_TICKS) {
            spaceship.temp = SpaceshipDisplay::NUM_BLANK;
        } else {
            state.tempSensorUnavailableTicks++;
        }
    }
    spaceship.audioButton = audioButtons.getPressedButton();
    spaceship.hftButton = hftButtons.getPressedButton();
    spaceship.reversing = !reversingSensor.read();
}

static bool emitEvents(SpaceshipState &prevState) {
    auto curState = state.spaceship;
    if (curState.reversing != prevState.reversing) {
        reply.event = ConnectorEvent::REVERSING_CHANGED;
        reply.dataSize = 1;
        reply.data[0] = curState.reversing;
        if (!connection.writeReply(reply)) return false;
    }
    return true;
}

static void updateVirtualKeyboard(SpaceshipState &prevState) {
    auto curState = state.spaceship;
    if (curState.audioButton != prevState.audioButton) {
        setKeyPressed(MEDIA_NEXT, curState.audioButton == SpaceshipButtons::AUDIO_CH_PLUS);
        setKeyPressed(MEDIA_PREVIOUS, curState.audioButton == SpaceshipButtons::AUDIO_CH_MINUS);
    }
    if (curState.hftButton != prevState.hftButton) {
        setKeyPressed(MEDIA_PLAY_PAUSE, curState.hftButton == SpaceshipButtons::HFT_TALK);
    }
}

static void setKeyPressed(ConsumerKeycode key, bool value) {
    if (value) {
        Consumer.press(key);
    } else {
        Consumer.release(key);
    }
}

static bool processInputCommand() {
    if (!connection.readRequest(request)) return true;
    reply.event = ConnectorEvent::INVALID_COMMAND;
    reply.dataSize = 0;
    switch (request.command) {
        case ConnectorCommand::PING: {
            auto size = request.dataSize;
            reply.event = ConnectorEvent::COMMAND_RESULT;
            reply.dataSize = size;
            memcpy(reply.data, request.data, size);
            break;
        }
        case ConnectorCommand::SET_DATE_TIME: {
            if (request.dataSize == 7) {
                auto data = request.data;
                DateTime dt = {
                        {data[3], data[4], data[5], data[6]},
                        {data[0], data[1], data[2]}
                };
                if (dt.isValid()) {
                    state.dateTime = dt;
                    rtc.setWriteProtect(false);
                    rtc.setDateTime(dt);
                    reply.event = ConnectorEvent::COMMAND_RESULT;
                }
            }
            break;
        }
        case ConnectorCommand::SET_AUDIO_TITLE: {
            if (request.dataSize <= SpaceshipDisplay::Text::SIZE) {
                memcpy(state.audioTitle, request.data, request.dataSize);
                state.audioTitle[request.dataSize] = '\0';
            } else {
                memcpy(state.audioTitle, request.data, sizeof state.audioTitle);
                state.audioTitle[SpaceshipDisplay::Text::SIZE] = '\0';
            }
            reply.event = ConnectorEvent::COMMAND_RESULT;
            break;
        }
        case ConnectorCommand::SET_SETTINGS: {
            if (request.dataSize == sizeof(SavedSettings)) {
                SavedSettings *requestedSettings = reinterpret_cast<SavedSettings*>(request.data);
                Settings newSettings;
                if (requestedSettings->load(newSettings)) {
                    settings = newSettings;
                    RtcRamData newRamData = { state.dateTime, *requestedSettings };
                    rtc.writeRam(&newRamData, sizeof newRamData);
                    reply.event = ConnectorEvent::COMMAND_RESULT;
                }
            }
            break;
        }
        case ConnectorCommand::GET_REVERSING: {
            reply.event = ConnectorEvent::COMMAND_RESULT;
            reply.dataSize = 1;
            reply.data[0] = state.spaceship.reversing;
            break;
        }
    }
    return connection.writeReply(reply);
}

static void updateDisplay() {
    auto clock = display.clock;
    auto text = display.text;
    auto hvac = display.hvac;
    auto media = display.media;

    if (state.splashTicks > 0) {
        auto spinnerFrame = (state.tick / SPLASH_SPINNER_FRAME_PERIOD) % SpaceshipDisplay::NUM_SPINNER_FRAMES;
        auto value = SpaceshipDisplay::NUM_SPINNER + spinnerFrame;
        hvac.leftTemp(value, false);
        hvac.rightTemp(value, false);
        display.commitState(segmentDriver);
        if (--state.splashTicks == 0) {
            display.clearAll();
        }
        return;
    }

    if (audio.isSwitchedOn()) {
        display.segments.writeFrom(audio.getDisplay().segments);
        if (audio.isAuxModeDisplayed()) {
            if (*state.audioTitle) {
                display.text.clear();
                display.text.showString(0, state.audioTitle);
            } else {
                display.text.showString(0, F("AUX/USB"));
            }
        }
    } else {
        display.clearAll();
    }

    auto dateTime = state.dateTime;
    if (dateTime.isValid() && dateTime.date.year > 20) {
        clock.hour(dateTime.time.hour);
        clock.minute(dateTime.time.minute);
        clock.colon(state.tick % CLOCK_BLINK_PERIOD_TICKS < CLOCK_BLINK_PERIOD_TICKS / 2);

        if (!audio.isSwitchedOn()) {
            char buf[12];
            dateTime.date.format(buf, settings.dateFormat, *settings.dateLocale);
            if (settings.dateCaps) toUpperCp1251(buf);
            text.showString(0, buf);
        }
    } else {
        clock.hour(SpaceshipDisplay::NUM_DASH);
        clock.minute(SpaceshipDisplay::NUM_DASH);
        clock.colon(true);
    }

    auto &hvacState = state.spaceship.hvac;
    if (hvacState.connected) {
        uint8_t leftTemp;
        uint8_t rightTemp;
        if (hvacState.leftWheel) {
            leftTemp = hvacState.temp1;
            rightTemp = hvacState.temp2;
        } else {
            leftTemp = hvacState.temp2;
            rightTemp = hvacState.temp1;
        }
        hvac.leftAuto(hvacState.autoMode && leftTemp);
        hvac.leftTemp(renderHvacTemp(leftTemp));
        hvac.rightAuto(hvacState.autoMode && rightTemp);
        hvac.rightTemp(renderHvacTemp(rightTemp));
        hvac.fan(hvacState.fanSpeed != 0, hvacState.fanSpeed);
        hvac.ac(hvacState.acOn, hvacState.acOff);
        bool windshield = false;
        bool front = false;
        bool bottom = false;
        switch (hvacState.airDirection) {
            case 1:
                front = true;
                break;
            case 2:
                front = true;
                bottom = true;
                break;
            case 3:
                bottom = true;
                break;
            case 4:
                windshield = true;
                bottom = true;
                break;
            case 5:
                windshield = true;
        }
        hvac.mode(windshield, hvacState.airDirection != 0, front, bottom);
    } else {
        hvac.clear();
    }

    auto temp = state.spaceship.temp;
    if (temp != SpaceshipDisplay::NUM_BLANK) {
        if (temp > -100 && temp < -9) temp = SpaceshipDisplay::Hvac::TEMP_LO;
        if (temp > 99) temp = SpaceshipDisplay::Hvac::TEMP_HI;
        hvac.rightTemp(temp);
    }

    media.dolby(state.spaceship.reversing);
    media.tel(state.phoneConnected);

    display.commitState(segmentDriver);
}

static int8_t renderHvacTemp(uint8_t value) {
    switch (value) {
        case 0:
            return SpaceshipDisplay::NUM_BLANK;
        case SpaceshipHvac::TEMP_LO:
            return SpaceshipDisplay::Hvac::TEMP_LO;
        case SpaceshipHvac::TEMP_HI:
            return SpaceshipDisplay::Hvac::TEMP_HI;
        default:
            return value;
    }
}

static void toUpperCp1251(char *str) {
    while (auto ch = (uint8_t) *str) {
        if (ch >= 0x61 && ch <= 0x7a) ch -= 0x20;
        if (ch == 0xb8) ch = 0xa8;
        if (ch >= 0xe0) ch -= 0x20;
        *str++ = (char) ch;
    }
}

// SPI interrupt routine
ISR (SPI_STC_vect) {
    audio.onSpiTransferFinished();
}
