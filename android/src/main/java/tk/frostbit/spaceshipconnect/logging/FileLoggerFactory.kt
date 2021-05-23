package tk.frostbit.spaceshipconnect.logging

import kotlinx.coroutines.*
import kotlinx.coroutines.channels.Channel
import ru.kontur.kinfra.logging.LoggerFactory
import ru.kontur.kinfra.logging.backend.LoggerBackend
import ru.kontur.kinfra.logging.backend.LoggingRequest
import java.io.File
import java.io.FileOutputStream
import java.io.PrintWriter
import java.time.Instant
import java.time.ZoneId
import java.time.format.DateTimeFormatter

class FileLoggerFactory(
    override val delegate: LoggerFactory,
    file: File
) : LoggerFactory.Wrapper() {

    private val logEvents = Channel<LogEvent>(Channel.UNLIMITED)
    private val writer = PrintWriter(FileOutputStream(file, true).bufferedWriter(), true)

    init {
        GlobalScope.launch(CoroutineName("File logger")) { loggingLoop() }
    }

    override fun getLoggerBackend(name: String): LoggerBackend {
        val delegateBackend = super.getLoggerBackend(name)
        return TeeLoggerBackend(delegateBackend) { log(name, it) }
    }

    private fun log(loggerName: String, request: LoggingRequest) {
        val event = LogEvent(Instant.now(), loggerName, request)
        logEvents.trySend(event)
    }

    private suspend fun loggingLoop() {
        withContext(Dispatchers.IO) {
            for (event in logEvents) {
                with(writer) {
                    timeFormat.formatTo(event.time.atZone(ZoneId.systemDefault()), this)
                    append(' ')
                    append(event.request.level.name[0])
                    append('/')
                    append(event.loggerName.substringAfterLast('.'))
                    append(": ")
                    append(event.request.decoratedMessage)
                    event.request.additionalData.throwable?.let { error ->
                        appendLine()
                        error.printStackTrace(this)
                    }
                    println()
                }
            }
        }
    }

    private data class LogEvent(
        val time: Instant,
        val loggerName: String,
        val request: LoggingRequest,
    )

    companion object {
        private val timeFormat = DateTimeFormatter.ofPattern("yy-MM-dd HH:mm:ss.SSS")
    }

}
