#pragma once

namespace config {
constexpr char WIFI_SSID[] = "";
constexpr char WIFI_PASSWORD[] = "";

constexpr char TELEGRAM_BOT_TOKEN[] = "";
constexpr char TELEGRAM_CHAT_ID[] = "";

constexpr char DEVICE_NAME[] = "co2-monitor";
constexpr unsigned long SAVE_INTERVAL_SECONDS = 60UL;
constexpr unsigned long HISTORY_RETENTION_ROWS = 1440;
constexpr uint16_t CO2_THRESHOLD_PPM = 1200;
constexpr uint16_t ALERT_STEP_PPM = 200;
constexpr unsigned long TELEGRAM_COOLDOWN_SECONDS = 30UL * 60UL;
constexpr bool STORE_TEMPERATURE = true;
constexpr bool TELEGRAM_ENABLED = true;
constexpr char TELEGRAM_MESSAGE_TEMPLATE[] =
    "CO2 alert: {co2_ppm} ppm\nTemperature: {temperature_c} C";
}
