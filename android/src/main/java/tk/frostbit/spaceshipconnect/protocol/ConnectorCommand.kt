package tk.frostbit.spaceshipconnect.protocol

import java.time.LocalDateTime

sealed class ConnectorCommand<T>(
    protected val code: ConnectorCommandCode,
) {

    fun toRequest(): ConnectorRequest {
        return ConnectorRequest(code, serializeData())
    }

    protected abstract fun serializeData(): ByteArray

    abstract fun parseResult(replyData: ByteArray): T

    override fun toString(): String {
        return javaClass.simpleName
    }

    class Ping(val data: ByteArray) : ConnectorCommand<ByteArray>(ConnectorCommandCode.PING) {
        override fun serializeData() = data
        override fun parseResult(replyData: ByteArray) = replyData
    }

    data class SetDateTime(val dateTime: LocalDateTime): ConnectorCommand<Unit>(ConnectorCommandCode.SET_DATE_TIME) {
        override fun serializeData(): ByteArray {
            return with(dateTime) {
                byteArrayOf(
                    hour.toByte(),
                    minute.toByte(),
                    second.toByte(),
                    dayOfWeek.value.toByte(),
                    dayOfMonth.toByte(),
                    month.value.toByte(),
                    year.minus(2000).toByte(),
                )
            }
        }
        override fun parseResult(replyData: ByteArray) = Unit
    }

    data class SetSettings(val settings: ConnectorSettings): ConnectorCommand<Unit>(ConnectorCommandCode.SET_SETTINGS) {
        override fun serializeData(): ByteArray {
            return with(settings) {
                byteArrayOf(
                    dateLocaleIndex.toByte(),
                    dateFormatIndex.toByte(),
                    if (dateCaps) 1 else 0,
                )
            }
        }
        override fun parseResult(replyData: ByteArray) = Unit
    }

    data class SetAudioTitle(val title: String): ConnectorCommand<Unit>(ConnectorCommandCode.SET_AUDIO_TITLE) {
        override fun serializeData(): ByteArray {
            return title.toByteArray(ConnectorConstants.CHARSET)
        }
        override fun parseResult(replyData: ByteArray) = Unit
    }

    object IsReversing : ConnectorCommand<Boolean>(ConnectorCommandCode.GET_REVERSING) {
        override fun serializeData(): ByteArray = byteArrayOf()
        override fun parseResult(replyData: ByteArray) = replyData[0] != 0.toByte()
    }

}
