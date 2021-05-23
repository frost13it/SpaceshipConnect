package tk.frostbit.spaceshipconnect

import android.app.Application
import ru.kontur.kinfra.logging.DefaultLoggerFactory
import ru.kontur.kinfra.logging.Logger
import tk.frostbit.logging.logcat.LogcatLoggerFactory
import tk.frostbit.spaceshipconnect.logging.FileLoggerFactory

class SpaceshipConnectApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        System.setProperty("kotlinx.coroutines.debug", "on")
        DefaultLoggerFactory.delegate = FileLoggerFactory(
            delegate = LogcatLoggerFactory(),
            file = getExternalFilesDir(null)!!.resolve("app.log")
        )
        Logger.currentClass().debug { "Application created" }
    }

}
