package tk.frostbit.spaceshipconnect.logging

import ru.kontur.kinfra.logging.LogLevel
import ru.kontur.kinfra.logging.backend.LoggerBackend
import ru.kontur.kinfra.logging.backend.LoggingRequest

internal class TeeLoggerBackend(
    private val delegate: LoggerBackend,
    private val teeLog: (LoggingRequest) -> Unit,
) : LoggerBackend {

    override fun isEnabled(level: LogLevel): Boolean {
        return delegate.isEnabled(level)
    }

    override fun log(request: LoggingRequest) {
        delegate.log(request)
        teeLog(request)
    }

}
