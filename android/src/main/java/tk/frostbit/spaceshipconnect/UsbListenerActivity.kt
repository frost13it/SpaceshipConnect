package tk.frostbit.spaceshipconnect

import android.app.Activity
import android.content.Intent
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import android.os.Bundle
import ru.kontur.kinfra.logging.Logger

class UsbListenerActivity : Activity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val device: UsbDevice? = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE)
        if (device != null) {
            logger.debug { "Starting service for device: $device" }

            val serviceIntent = Intent(this, SpaceshipConnectionService::class.java).apply {
                putExtra(UsbManager.EXTRA_DEVICE, device)
            }
            startForegroundService(serviceIntent)
        } else {
            logger.error { "No device supplied in intent" }
        }

        finish()
    }

    companion object {

        private val logger = Logger.currentClass()

    }

}
