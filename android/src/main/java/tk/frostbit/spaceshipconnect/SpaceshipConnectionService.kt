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
import kotlin.coroutines.CoroutineContext

class SpaceshipConnectionService : Service() {

    private val notificationManager by lazy { getSystemService<NotificationManager>()!! }
    private val mediaSessionManager by lazy { getSystemService<MediaSessionManager>()!! }
    private val notificationChannel by lazy { getMainNotificationChannel() }
    private val coroutineScope = CoroutineScope(
        SupervisorJob() +
                Dispatchers.Main +
                CoroutineExceptionHandler(this::handleException)
    )

    private val detachReceiver = DetachReceiver()
    private val mediaSessionListener = MediaSessionListener()

    private lateinit var device: UsbDevice
    private var connection: SpaceshipConnection? = null
    private var closed = false

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        device = intent?.getParcelableExtra(UsbManager.EXTRA_DEVICE) ?: run {
            stopSelf()
            return START_NOT_STICKY
        }

        startForeground(
            NOTIFICATION_ID,
            buildNotification("Connecting"),
            ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE
        )

        coroutineScope.launch {
            openConnection(device)

            connection?.let { connection ->
                launch { connection.loop() }
                connection.validate()
                afterConnect(connection)
                launch { audioTitleUpdateLoop() }
            }

            notificationManager.notify(NOTIFICATION_ID, buildNotification("Connected"))
            Toast.makeText(this@SpaceshipConnectionService, "Spaceship connected", Toast.LENGTH_SHORT).show()
        }

        registerReceiver(detachReceiver, IntentFilter(UsbManager.ACTION_USB_DEVICE_DETACHED))

        val notificationListenerName = ComponentName.createRelative(this, ".NotificationListenerService")
        mediaSessionManager.addOnActiveSessionsChangedListener(mediaSessionListener, notificationListenerName)
        mediaSessionListener.setActiveSessions(mediaSessionManager.getActiveSessions(notificationListenerName))

        return START_REDELIVER_INTENT
    }

    private suspend fun openConnection(device: UsbDevice) {
        val connection = SpaceshipConnection.open(device, this)
        if (closed) {
            connection.close()
        } else {
            this.connection = connection
        }
    }

    private suspend fun afterConnect(connection: SpaceshipConnection) {
        connection.execCommand(ConnectorCommand.SetDateTime(LocalDateTime.now()))

        val settings = ConnectorSettings(
            dateLocaleIndex = LocaleList.getDefault()
                .toLanguageTags().split(",")
                .indexOfFirst { preferred ->
                    ConnectorSettings.DATE_LOCALES.any { preferred.startsWith(it) }
                }
                .let { if (it == -1) 1 else it },
            dateFormatIndex = 1,
            dateCaps = true,
        )
        logger.info { "Applying settings: $settings" }
        connection.execCommand(ConnectorCommand.SetSettings(settings))
    }

    private suspend fun audioTitleUpdateLoop() {
        mediaSessionListener.currentTitle.collect { title ->
            logger.info { "Updating audio title: $title" }
            connection?.execCommand(ConnectorCommand.SetAudioTitle(title.toUpperCase(Locale.ROOT)))
        }
    }

    private fun handleException(context: CoroutineContext, throwable: Throwable) {
        logger.error(throwable) { "Connection failure" + context[CoroutineName]?.let { " in $it" }.orEmpty() }
        Toast.makeText(this@SpaceshipConnectionService, "Spaceship disconnected", Toast.LENGTH_LONG).show()
        close("failure")
    }

    private fun buildNotification(title: String, text: String? = null): Notification {
        return Notification.Builder(this, notificationChannel.id)
            .setSmallIcon(R.drawable.ic_auto)
            .setContentTitle(title)
            .setContentText(text)
            .setVisibility(Notification.VISIBILITY_PUBLIC)
            .build()
    }

    private fun getMainNotificationChannel(): NotificationChannel {
        return notificationManager.getNotificationChannel(NOTIFICATION_CHANNEL_ID) ?: run {
            NotificationChannel(NOTIFICATION_CHANNEL_ID, "Connection", NotificationManager.IMPORTANCE_LOW).also {
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
        coroutineScope.cancel(message)
        GlobalScope.launch {
            connection?.close()
        }
        notificationManager.notify(NOTIFICATION_ID, buildNotification("Disconnected"))
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
        private const val NOTIFICATION_ID = NotificationIds.CONNECTION
        private const val NOTIFICATION_CHANNEL_ID = "connection"

    }

}
