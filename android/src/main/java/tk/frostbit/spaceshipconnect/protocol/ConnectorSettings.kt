package tk.frostbit.spaceshipconnect.protocol

data class ConnectorSettings(
    val dateLocaleIndex: Int,
    val dateFormatIndex: Int,
    val dateCaps: Boolean,
    val tempSensorIndex: Int,
) {

    companion object {

        val DATE_LOCALES = listOf("ru", "en")

    }

}
