package tk.frostbit.spaceshipconnect

import android.service.notification.NotificationListenerService
import android.util.Log

class NotificationListenerService : NotificationListenerService() {

    override fun onListenerConnected() {
        Log.i(TAG, "Listener connected")
    }

    override fun onListenerDisconnected() {
        Log.i(TAG, "Listener disconnected")
    }

    companion object {

        private const val TAG = "NotificationListenerService"

    }

}
