package tk.frostbit.spaceshipconnect

import android.content.Context
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import androidx.core.content.getSystemService
import com.hoho.android.usbserial.driver.CdcAcmSerialDriver
import com.hoho.android.usbserial.driver.UsbSerialPort
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import ru.kontur.kinfra.logging.Logger
import tk.frostbit.spaceshipconnect.protocol.*

class SpaceshipConnection(private val port: UsbSerialPort) {

    @Volatile
    private var pendingResult: CompletableDeferred<ByteArray>? = null
    private val commandMutex = Mutex()

    private val _events = MutableSharedFlow<ConnectorEvent>()
    val events: Flow<ConnectorEvent>
        get() = _events

    suspend fun <T> execCommand(command: ConnectorCommand<T>): T = commandMutex.withLock {
        check(pendingResult == null) { "A command already in progress" }
        val resultDeferred = CompletableDeferred<ByteArray>()
        pendingResult = resultDeferred
        withContext(Dispatchers.IO) {
            logger.debug { "Sending command $command" }
            writeRequest(command.toRequest())
        }
        val result = withTimeoutOrNull(REQUEST_TIMEOUT.toLong()) {
            resultDeferred.await()
        }
        return command.parseResult(result ?: throw RuntimeException("Command $command timed out"))
    }

    suspend fun validate() {
        val testData = byteArrayOf(0, -1, 127, -128)
        val result = execCommand(ConnectorCommand.Ping(testData))
        if (!result.contentEquals(testData)) {
            throw RuntimeException("Invalid ping response: ${result.contentToString()}")
        }
    }

    suspend fun loop() {
        withContext(CoroutineName("SpaceshipConnection Looper") + Dispatchers.IO) {
            logger.debug { "Listening port" }
            while (coroutineContext.isActive) {
                val reply = readNextReply()
                if (reply == null) {
                    delay(50)
                    continue
                }

                logger.debug { "Received reply: $reply" }

                when (reply.event) {
                    ConnectorEventCode.COMMAND_RESULT -> {
                        pendingResult?.complete(reply.data)
                        pendingResult = null
                    }

                    ConnectorEventCode.INVALID_COMMAND -> {
                        pendingResult?.completeExceptionally(RuntimeException("Connector rejected the command"))
                    }

                    else -> _events.emit(ConnectorEvent.parse(reply))
                }
            }
            logger.debug { "Listening finished" }
        }
    }

    suspend fun close() {
        withContext(Dispatchers.IO) {
            try {
                port.dtr = false
            } catch (e: Exception) {
                logger.error { "Failed to reset DTR" }
            }
            port.close()
        }
    }

    private fun writeRequest(request: ConnectorRequest) {
        require(request.data.size <= ConnectorPacket.MAX_DATA_SIZE) {
            "Packet data size overflow: ${request.data.size}"
        }

        val buffer = ByteArray(request.data.size + 2)
        buffer[0] = request.command.value.toByte()
        buffer[1] = request.data.size.toByte()
        request.data.copyInto(buffer, 2)
        port.write(buffer, REQUEST_WRITE_TIMEOUT).also {
            if (it != buffer.size) {
                logger.error { "Incomplete write: $it of ${buffer.size} bytes" }
            }
        }
    }

    private fun readNextReply(): ConnectorReply? {
        val buffer = ByteArray(16_384)
        val readCount = port.read(buffer, REPLY_READ_TIMEOUT).also {
            if (it < PACKET_HEADER_SIZE) {
                if (it > 0) logger.warn { "Read $it of $PACKET_HEADER_SIZE header bytes" }
                return null
            }
        }
        val type = buffer[0].toInt() and 0xff
        val dataSize = buffer[1].toInt()
        val packetSize = PACKET_HEADER_SIZE + dataSize
        val event = ConnectorEventCode.fromValue(type) ?: run {
            logger.error { "Unknown event: $type" }
            return null
        }
        val data = buffer.copyOfRange(PACKET_HEADER_SIZE, packetSize)
        if (readCount > packetSize) {
            logger.warn { "Discarded ${readCount - packetSize} bytes" }
        }
        return ConnectorReply(event, data)
    }

    companion object {

        private val logger = Logger.currentClass()

        private const val PACKET_HEADER_SIZE = 2
        private const val REPLY_READ_TIMEOUT = 150
        private const val REQUEST_WRITE_TIMEOUT = 150
        private const val REQUEST_TIMEOUT = 500

        suspend fun open(device: UsbDevice, context: Context): SpaceshipConnection {
            val manager = context.getSystemService<UsbManager>()!!
            val deviceConnection = manager.openDevice(device)
            val port = CdcAcmSerialDriver(device).ports.single().apply {
                withContext(Dispatchers.IO) {
                    open(deviceConnection)
                    rts = true
                    dtr = true
                }
            }
            return SpaceshipConnection(port)
        }

    }

}
