#pragma once

#include <Arduino.h>

#include "protocol.h"

class Connection {

    static Serial_ &port;

public:

    bool readRequest(ConnectorRequest &outRequest);

    bool writeReply(ConnectorReply &reply);

};
