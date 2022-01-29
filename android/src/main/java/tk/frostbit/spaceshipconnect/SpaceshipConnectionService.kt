package tk.frostbit.spaceshipconnect

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.*
import android.content.pm.ServiceInfo
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import android.media.AudioAttributes
import android.media.MediaMetadata
import android.media.session.MediaController
import android.media.session.MediaSessionManager
import android.media.session.PlaybackState
import android.os.IBinder
import android.os.LocaleList
import android.widget.Toast
import androidx.core.content.getSystemService
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.*
import ru.kontur.kinfra.logging.Logger
import tk.frostbit.spaceshipconnect.protocol.ConnectorCommand
import tk.frostbit.spaceshipconnect.protocol.ConnectorConstants
import tk.frostbit.spaceshipconnect.protocol.ConnectorSettings
import java.time.LocalDateTime
import java.util.*

class SpaceshipConnectionService : Service() {

    private val notificationManager by lazy { getSystemService<NotificationManager>()!! }
    private val mediaSessionManager by lazy { getSystemService<MediaSessionManager>()!! }
    private val usbManager by lazy { getSystemService<UsbManager>()!! }
    private val notificationChannel by lazy { getMainNotificationChannel() }
    private val serviceCoroutineScope = CoroutineScope(SupervisorJob() + Dispatchers.Main)

    private val detachReceiver = DetachReceiver()
    private val mediaSessionListener = MediaSessionListener()

    private lateinit var device: UsbDevice
    private var connection: SpaceshipConnection? = null
    private var closed = false
    private var statusToast: Toast? = null

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        device = intent?.getParcelableExtra(UsbManager.EXTRA_DEVICE) ?: run {
            stopSelf()
            return START_NOT_STICKY
        }

        startForeground(
            STATUS_NOTIFICATION_ID,
            buildStatusNotification("Connecting"),
            ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE
        )

        serviceCoroutineScope.launch {
            var attempt = 1
            while (true) {
                logger.info { "Connecting (attempt: $attempt)" }
                showStatusNotification("Connecting")
                try {
                    coroutineScope {
                        val connectionScope = this
                        val connection = SpaceshipConnection.open(device, usbManager, connectionScope)
                        connection.validate()
                        afterConnect(connection, connectionScope)
                        logger.info { "Connected" }
                        showStatusNotification("Connected")
                        showStatusToast("Spaceship connected")
                        this@SpaceshipConnectionService.connection = connection
                        awaitCancellation()
                    }
                } catch (e: Exception) {
                    if (e !is CancellationException && !closed) {
                        logger.error(e) { "Connection failed" }
                    }
                    showStatusNotification("Disconnected")
                    showStatusToast("Spaceship disconnected")
                    connection?.close()
                    connection = null
                }
                attempt++
                delay(500)
            }
        }

        registerReceiver(detachReceiver, IntentFilter(UsbManager.ACTION_USB_DEVICE_DETACHED))

        val notificationListenerName = ComponentName.createRelative(this, ".NotificationListenerService")
        mediaSessionManager.addOnActiveSessionsChangedListener(mediaSessionListener, notificationListenerName)
        mediaSessionListener.setActiveSessions(mediaSessionManager.getActiveSessions(notificationListenerName))

        return START_REDELIVER_INTENT
    }

    private suspend fun afterConnect(connection: SpaceshipConnection, scope: CoroutineScope) {
        connection.execCommand(ConnectorCommand.SetDateTime(LocalDateTime.now()))

        val settings = ConnectorSettings(
            dateLocaleIndex = LocaleList.getDefault()
                .toLanguageTags()
                .split(",")
                .map { Locale.forLanguageTag(it).language }
                .map { ConnectorSettings.DATE_LOCALES.indexOf(it) }
                .firstOrNull { it != -1 }
                ?: 0,
            dateFormatIndex = 1,
            dateCaps = true,
        )
        logger.info { "Applying settings: $settings" }
        connection.execCommand(ConnectorCommand.SetSettings(settings))

        mediaSessionListener.currentTitle
            .onEach { title ->
                logger.info { "Updating audio title: '$title'" }
                connection.execCommand(ConnectorCommand.SetAudioTitle(title.toUpperCase(Locale.ROOT)))
            }
            .launchIn(scope)
    }

    private fun showStatusToast(text: String) {
        statusToast?.cancel()
        statusToast = Toast.makeText(this, text, Toast.LENGTH_SHORT).apply { show() }
    }

    private fun showStatusNotification(title: String, text: String? = null) {
        val notification = buildStatusNotification(title, text)
        notificationManager.notify(STATUS_NOTIFICATION_ID, notification)
    }

    private fun buildStatusNotification(title: String, text: String? = null): Notification {
        return Notification.Builder(this, notificationChannel.id)
            .setSmallIcon(R.drawable.ic_auto)
            .setContentTitle(title)
            .setContentText(text)
            .setVisibility(Notification.VISIBILITY_PUBLIC)
            .build()
    }

    private fun getMainNotificationChannel(): NotificationChannel {
        return notificationManager.getNotificationChannel(STATUS_NOTIFICATION_CHANNEL_ID) ?: run {
            NotificationChannel(
                STATUS_NOTIFICATION_CHANNEL_ID,
                "Connection status",
                NotificationManager.IMPORTANCE_LOW
            ).also {
                notificationManager.createNotificationChannel(it)
            }
        }
    }

    private fun close(message: String) {
        if (closed) return
        closed = true

        logger.info { "Connection is closed ($message)" }
        unregisterReceiver(detachReceiver)
        mediaSessionManager.removeOnActiveSessionsChangedListener(mediaSessionListener)
        serviceCoroutineScope.cancel(message)
        stopSelf()
    }

    override fun onDestroy() {
        close("service destroyed")
    }

    override fun onBind(intent: Intent): IBinder? = null

    private inner class DetachReceiver : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val detachedDevice = intent.getParcelableExtra<UsbDevice>(UsbManager.EXTRA_DEVICE) ?: return
            if (detachedDevice.deviceId == device.deviceId) {
                logger.info { "Detaching from device ${device.deviceId}" }
                close("device detached")
            }
        }
    }

    private class MediaSessionListener : MediaSessionManager.OnActiveSessionsChangedListener {

        private val controllerCallback = ControllerCallback()
        private val controllers = MutableStateFlow(emptyList<MediaController>())
        private val updates = MutableStateFlow(UpdateCookie())
        val currentTitle: Flow<String> = controllers.combine(updates) { controllers, _ -> controllers }
            .map { findPlayingTrack(it).orEmpty() }
            .distinctUntilChanged()

        fun setActiveSessions(controllers: List<MediaController>) {
            for (controller in controllers) {
                controller.registerCallback(controllerCallback)
            }
            this.controllers.value = controllers
        }

        override fun onActiveSessionsChanged(controllers: List<MediaController>?) {
            logger.debug { "Media sessions updated: $controllers" }
            setActiveSessions(controllers.orEmpty())
        }

        private fun findPlayingTrack(controllers: List<MediaController>): String? {
            return controllers
                .filter { it.playbackState?.state == PlaybackState.STATE_PLAYING }
                .filter { it.playbackInfo?.audioAttributes?.usage == AudioAttributes.USAGE_MEDIA }
                .firstOrNull()
                ?.metadata
                ?.let { getTrackTitle(it) }
        }

        private fun getTrackTitle(metadata: MediaMetadata): String? {
            val title = (metadata.getString(MediaMetadata.METADATA_KEY_TITLE) ?: return null)
                .substringBefore('(')
                .substringBefore('[')
                .substringBefore(" feat.")
                .substringBefore(" ft.")
                .trim()
            val artist = metadata.getString(MediaMetadata.METADATA_KEY_ARTIST)
                ?.takeUnless { it == "Unknown artist" }
            val summary = if (artist != null) {
                "$title - $artist"
            } else {
                title
            }
            return summary
                .replace(" - ", "-")
                .take(ConnectorConstants.TEXT_SECTION_SIZE)
        }

        private fun update() {
            updates.value = UpdateCookie()
        }

        private class UpdateCookie

        private inner class ControllerCallback : MediaController.Callback() {
            override fun onPlaybackStateChanged(state: PlaybackState?) {
                logger.info { "Playback state changed" }
                update()
            }

            override fun onMetadataChanged(metadata: MediaMetadata?) {
                logger.info { "Metadata changed" }
                update()
            }

            override fun onAudioInfoChanged(info: MediaController.PlaybackInfo?) {
                logger.info { "Audio info changed" }
                update()
            }
        }

    }

    companion object {

        private val logger = Logger.currentClass()
        private const val STATUS_NOTIFICATION_ID = NotificationIds.CONNECTION_STATUS
        private const val STATUS_NOTIFICATION_CHANNEL_ID = "connection"

    }

}
