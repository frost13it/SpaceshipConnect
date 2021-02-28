package tk.frostbit.spaceshipconnect

import android.app.Activity
import android.content.Intent
import android.hardware.camera2.CameraManager
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import android.os.Bundle
import android.util.Log
import androidx.core.content.getSystemService

class UsbListenerActivity : Activity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val device: UsbDevice? = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE)
        if (device != null) {
            Log.d(TAG, "Starting service for device: $device")

            val serviceIntent = Intent(this, SpaceshipConnectionService::class.java).apply {
                putExtra(UsbManager.EXTRA_DEVICE, device)
            }
            startForegroundService(serviceIntent)
        } else {
            Log.e(TAG, "No device supplied in intent")
        }

        finish()
    }

    companion object {

        private const val TAG = "UsbActivity"

    }

}
