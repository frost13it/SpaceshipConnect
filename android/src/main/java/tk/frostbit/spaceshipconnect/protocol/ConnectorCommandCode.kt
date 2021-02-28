package tk.frostbit.spaceshipconnect.protocol

enum class ConnectorCommandCode(val value: Int) {
    PING(0),
    SET_DATE_TIME(1),
    SET_SETTINGS(2),
    GET_REVERSING(100),
}
