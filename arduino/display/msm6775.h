#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../util/Pin.h"

namespace msm6775 {

    class SegmentDriver {

    public:

        SegmentDriver(uint8_t clockPin, uint8_t dataPin, uint8_t loadPin) : clockPin(clockPin),
                                                                            dataPin(dataPin),
                                                                            loadPin(loadPin) {
            configurePins();
        }

        void write(uint8_t *data, size_t size);

        /** Write a byte bit by bit, starting with LSB */
        void writeByte(uint8_t value);

        void load();

    private:

        const Pin clockPin;
        const Pin dataPin;
        const Pin loadPin;

        void configurePins();

    };

    class Emulator;

    class SegmentsState {

        friend class Emulator;

    private:

        // hardcoded for 1/4 duty mode, could be configurable
        static constexpr uint8_t BLOCKS_COUNT = 4;

    public:

        static constexpr uint8_t BLOCK_SEGMENTS_COUNT = 80;

        // Max supported number of segments
        static constexpr uint16_t SEGMENTS_COUNT = BLOCK_SEGMENTS_COUNT * BLOCKS_COUNT;

        void writeSegment(uint16_t index, bool value);

        void writeAll(bool value);

        void writeFrom(const SegmentsState &other);

        bool readSegment(uint16_t index);

        void commit(SegmentDriver &driver);

    private:

        static constexpr uint8_t BLOCK_STATE_SIZE = BLOCK_SEGMENTS_COUNT / 8;
        static constexpr uint8_t STATE_SIZE = BLOCK_STATE_SIZE * BLOCKS_COUNT;

        uint8_t state[STATE_SIZE] = {0};

    };

    class Emulator {

        static constexpr uint8_t BLOCK_STATE_SIZE = SegmentsState::BLOCK_STATE_SIZE;
        static constexpr uint8_t PACKET_SIZE = BLOCK_STATE_SIZE + 1;
        static constexpr uint8_t ALL_BLOCKS_MASK = (1 << SegmentsState::BLOCKS_COUNT) - 1;

        static Emulator *instance;

        volatile SegmentsState &segments;
        volatile uint8_t packet[PACKET_SIZE];
        volatile uint8_t packetPosition = 0;
        volatile uint16_t stateVersion = 0;

        void configureHardware(uint8_t loadInterruptPin);

        void onLoad();

        static void onLoad_ISR();

    public:

        Emulator(SegmentsState &segments, uint8_t loadPin) : segments(segments) {
            instance = this;
            configureHardware(loadPin);
        }

        uint16_t getStateVersion();

        void onSpiTransferFinished();

    };

}
