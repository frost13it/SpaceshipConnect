#pragma once

#include <stdint.h>

static constexpr uint8_t MAX_DATA_SIZE = 64 - 2;

struct ConnectorPacket {
    uint8_t dataSize;
    uint8_t data[MAX_DATA_SIZE];
};

enum class ConnectorCommand : uint8_t {
    PING = 0,
    SET_DATE_TIME = 1,
    SET_SETTINGS = 2,
    SET_AUDIO_TITLE = 3,
    GET_REVERSING = 100,
};

struct ConnectorRequest : ConnectorPacket {
    ConnectorCommand command;
};

enum class ConnectorEvent : uint8_t {
    COMMAND_RESULT = 0,
    INVALID_COMMAND = 1,
    REVERSING_CHANGED = 100,
};

struct ConnectorReply : ConnectorPacket {
    ConnectorEvent event;
};
