package tk.frostbit.spaceshipconnect.protocol

sealed class ConnectorEvent {

    data class ReversingChanged(val isReversing: Boolean) : ConnectorEvent()

    companion object {

        fun parse(reply: ConnectorReply): ConnectorEvent {
            val data = reply.data
            return when (val code = reply.event) {
                ConnectorEventCode.COMMAND_RESULT,
                ConnectorEventCode.INVALID_COMMAND -> throw IllegalArgumentException("Expected an event: $code")
                ConnectorEventCode.REVERSING_CHANGED -> ReversingChanged(data[0] != 0.toByte())
            }
        }

    }

}
