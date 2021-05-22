package tk.frostbit.spaceshipconnect

import android.app.Application
import ru.kontur.kinfra.logging.DefaultLoggerFactory
import tk.frostbit.logging.logcat.LogcatLoggerFactory

class SpaceshipConnectApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        System.setProperty("kotlinx.coroutines.debug", "on")
        DefaultLoggerFactory.delegate = LogcatLoggerFactory()
    }

}
