package tk.frostbit.spaceshipconnect.protocol

enum class ConnectorEventCode(val value: Int) {
    COMMAND_RESULT(0),
    INVALID_COMMAND(1),
    REVERSING_CHANGED(100),
    ;

    companion object {

        private val mapping = arrayOfNulls<ConnectorEventCode>(256).apply {
            for (event in values()) {
                set(event.value, event)
            }
        }

        fun fromValue(value: Int) = mapping[value]

    }

}
