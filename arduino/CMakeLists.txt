cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/arduino-cmake/cmake/ArduinoToolchain.cmake)
set(CMAKE_CXX_STANDARD 11)
set(COMPILE_FLAGS "-fexec-charset=cp1251 -finput-charset=utf-8 -O3")
set(PROJECT_NAME SpaceshipConnector)
set(ARDUINO_DEFAULT_BOARD uno)
set(ARDUINO_DEFAULT_PORT /dev/ttyUSB0)
#set(ARDUINO_DEBUG true)

project(${PROJECT_NAME})
add_definitions(-DUSB_PRODUCT="Spaceship Connector")

# For SparkFun board support
register_hardware_platform(${ARDUINO_SDK_PATH}/hardware/arduino/sparkfunavr)
set(ARDUINO_DEFAULT_BOARD promicro)
set(ARDUINO_DEFAULT_PORT /dev/ttyACM0)

set(HID_RECURSE True)
set(HID-Project_RECURSE True)

# Library

set(Spaceship_HDRS
        audio/SpaceshipAudio.h
        buttons/ButtonSet.h
        buttons/SpaceshipButtons.h
        clock/DateTime.h
        clock/ds1302.h
        connector/Connection.h
        connector/protocol.h
        connector/Settings.h
        connector/SpaceshipState.h
        display/SpaceshipDisplay.h
        display/msm6775.h
        hvac/SpaceshipHvac.h
        util/Pin.h
        util/CriticalSection.h
        util/TimedSection.h)

set(Spaceship_SRCS
        audio/SpaceshipAudio.cpp
        buttons/ButtonSet.cpp
        buttons/SpaceshipButtons.cpp
        clock/DateTime.cpp
        clock/ds1302.cpp
        connector/Connection.cpp
        connector/Settings.cpp
        display/SpaceshipDisplay.cpp
        display/section_clock.cpp
        display/section_text.cpp
        display/section_hvac.cpp
        display/section_media.cpp
        display/msm6775.cpp
        hvac/SpaceshipHvac.cpp
        util/Pin.cpp
        util/CriticalSection.cpp
        util/TimedSection.cpp)

generate_arduino_library(Spaceship)

# Main sketch

generate_arduino_firmware(main SKETCH SpaceshipConnector.ino LIBS Spaceship)

# Examples

generate_arduino_firmware(hvac_segments_test SKETCH examples/hvac_segments_test LIBS Spaceship)
generate_arduino_firmware(text_font_editor SKETCH examples/text_font_editor LIBS Spaceship)
generate_arduino_firmware(segment_walker SKETCH examples/segment_walker LIBS Spaceship)
generate_arduino_firmware(display_proxy SKETCH examples/display_proxy LIBS Spaceship)
generate_arduino_firmware(rtc_test SKETCH examples/rtc_test LIBS Spaceship)
