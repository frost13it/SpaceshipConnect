package tk.frostbit.spaceshipconnect.protocol

sealed class ConnectorPacket {

    abstract val data: ByteArray

    companion object {

        const val MAX_DATA_SIZE = 64 - 2;

    }

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
