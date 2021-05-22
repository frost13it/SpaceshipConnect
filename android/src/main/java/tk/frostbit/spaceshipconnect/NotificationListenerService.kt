package tk.frostbit.spaceshipconnect

import android.service.notification.NotificationListenerService
import ru.kontur.kinfra.logging.Logger

class NotificationListenerService : NotificationListenerService() {

    override fun onListenerConnected() {
        logger.info { "Listener connected" }
    }

    override fun onListenerDisconnected() {
        logger.info { "Listener disconnected" }
    }

    companion object {

        private val logger = Logger.currentClass()

    }

}
