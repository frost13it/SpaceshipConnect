package tk.frostbit.spaceshipconnect.protocol

sealed class ConnectorPacket {

    abstract val data: ByteArray

}

class ConnectorRequest(
    val command: ConnectorCommandCode,
    override val data: ByteArray,
) : ConnectorPacket() {

    override fun toString(): String {
        return "{command: $command, data: ${data.size} bytes}"
    }
}

class ConnectorReply(
    val event: ConnectorEventCode,
    override val data: ByteArray,
) : ConnectorPacket() {

    override fun toString(): String {
        return "{event: $event, data: ${data.size} bytes}"
    }

}
