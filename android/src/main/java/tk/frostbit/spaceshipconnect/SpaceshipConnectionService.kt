package tk.frostbit.spaceshipconnect

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.ServiceInfo
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import android.os.IBinder
import android.os.LocaleList
import android.util.Log
import android.widget.Toast
import androidx.core.content.getSystemService
import kotlinx.coroutines.*
import tk.frostbit.spaceshipconnect.protocol.ConnectorCommand
import tk.frostbit.spaceshipconnect.protocol.ConnectorSettings
import java.time.LocalDateTime
import java.util.*
import kotlin.coroutines.CoroutineContext

class SpaceshipConnectionService : Service() {

    private val notificationManager by lazy { getSystemService<NotificationManager>()!! }
    private val notificationChannel by lazy { getMainNotificationChannel() }
    private val coroutineScope = CoroutineScope(
        SupervisorJob() +
                Dispatchers.Main +
                CoroutineExceptionHandler(this::handleException)
    )

    private val detachReceiver = DetachReceiver()

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
            }

            notificationManager.notify(NOTIFICATION_ID, buildNotification("Connected"))
            Toast.makeText(this@SpaceshipConnectionService, "Spaceship connected", Toast.LENGTH_SHORT).show()
        }

        registerReceiver(detachReceiver, IntentFilter(UsbManager.ACTION_USB_DEVICE_DETACHED))

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
            tempSensorIndex = 0,
        )
        connection.execCommand(ConnectorCommand.SetSettings(settings))
    }

    private fun handleException(context: CoroutineContext, throwable: Throwable) {
        Log.e(TAG, "Connection failure" + context[CoroutineName]?.let { " in $it" }.orEmpty(), throwable)
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

        Log.i(TAG, "Connection is closed ($message)")
        unregisterReceiver(detachReceiver)
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
                Log.i(TAG, "Detaching from device ${device.deviceId}")
                close("device detached")
            }
        }
    }

    companion object {

        private const val TAG = "UsbConnectionService"
        private const val NOTIFICATION_ID = NotificationIds.CONNECTION
        private const val NOTIFICATION_CHANNEL_ID = "connection"

    }

}
