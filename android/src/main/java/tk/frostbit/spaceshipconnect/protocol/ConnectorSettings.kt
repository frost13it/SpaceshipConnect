package tk.frostbit.spaceshipconnect.protocol

data class ConnectorSettings(
    val dateLocaleIndex: Int,
    val dateFormatIndex: Int,
    val dateCaps: Boolean,
) {

    companion object {

        val DATE_LOCALES = listOf("en", "ru")

    }

}
