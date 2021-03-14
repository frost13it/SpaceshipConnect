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

    class SegmentDisplay {

    private:

        static constexpr uint8_t BLOCKS_COUNT = 4;

    public:

        static constexpr uint8_t BLOCK_SEGMENTS_COUNT = 80;

        // Max supported number of segments
        static constexpr uint16_t SEGMENTS_COUNT = BLOCK_SEGMENTS_COUNT * BLOCKS_COUNT;

        SegmentDisplay(uint8_t clockPin, uint8_t dataPin, uint8_t loadPin) : driver(clockPin, dataPin, loadPin) {};

        void writeSegment(uint16_t index, bool value);

        void writeAll(bool value);

        bool readSegment(uint16_t index);

        void commitState();

    private:

        static constexpr uint8_t BLOCK_STATE_SIZE = BLOCK_SEGMENTS_COUNT / 8;
        static constexpr uint8_t STATE_SIZE = BLOCK_STATE_SIZE * BLOCKS_COUNT;

        SegmentDriver driver;
        uint8_t state[STATE_SIZE] = {0};

    };

}
