package tk.frostbit.spaceshipconnect

import android.app.Application

class SpaceshipConnectApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        System.setProperty("kotlinx.coroutines.debug", "on")
    }
}
