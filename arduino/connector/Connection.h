#pragma once

#include <Arduino.h>

#include "protocol.h"

class Connection {

    static Stream &stream;

public:

    bool readRequest(ConnectorRequest &outRequest);

    bool writeReply(ConnectorReply &reply);

};
