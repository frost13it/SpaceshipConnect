#include "Connection.h"

Serial_ &Connection::port = Serial;

static uint8_t readImmediate(Serial_ &port, uint8_t *buf, uint8_t len) {
    uint8_t total = 0;
    while (len) {
        auto nextByte = port.read();
        if (nextByte < 0) break;
        *buf++ = nextByte;
        total++;
        len--;
    }
    return total;
}

bool Connection::readRequest(ConnectorRequest &outRequest) {
    uint8_t header[2];
    if (readImmediate(port, header, sizeof header) != sizeof header) {
        return false;
    }
    outRequest.command = static_cast<ConnectorCommand>(header[0]);

    auto dataSize = header[1];
    outRequest.dataSize = dataSize;

    return readImmediate(port, outRequest.data, dataSize) == dataSize;
}

bool Connection::writeReply(ConnectorReply &reply) {
    auto dataSize = reply.dataSize;
    auto packetSize = dataSize + 2;
    if (port.availableForWrite() < packetSize) return false;
    uint8_t packet[packetSize];
    packet[0] = static_cast<uint8_t>(reply.event);
    packet[1] = dataSize;
    memcpy(packet + 2, reply.data, dataSize);
    if (port.write(packet, packetSize) == packetSize) {
        port.flush();
        return true;
    } else {
        return false;
    }
}
