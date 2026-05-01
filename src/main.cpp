#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "app_config.h"
#include "config_store.h"
#include "dashboard_server.h"
#include "history_store.h"
#include "secrets.h"
#include "telegram_notifier.h"
#include "time_utils.h"
#include "zgm053u_sensor.h"

namespace {

Zgm053uSensor g_sensor(app_config::SENSOR_CLOCK_PIN, app_config::SENSOR_DATA_PIN);
ConfigStore g_config_store;
HistoryStore g_history_store(config::HISTORY_RETENTION_ROWS);
TelegramNotifier g_notifier(g_config_store);

SensorReading currentReading() {
  return g_sensor.latestReading();
}

DashboardServer g_dashboard(g_history_store, g_config_store, currentReading);
constexpr unsigned long kAlertCheckIntervalMs = 10000UL;
unsigned long g_last_sample_ms = 0;
unsigned long g_last_alert_check_ms = 0;
bool g_first_sample_pending = true;

constexpr uint8_t kWifiMaxAttempts = 40;

void onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t /*info*/) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("Wi-Fi connected. IP: ");
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      WiFi.reconnect();
      break;
    default:
      break;
  }
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.onEvent(onWifiEvent);

  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
  for (uint8_t attempt = 0; attempt < kWifiMaxAttempts && WiFi.status() != WL_CONNECTED; ++attempt) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
}

void setupTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP");
  for (int i = 0; i < 30 && !timeIsReady(); ++i) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);

  g_sensor.begin();

  if (!g_history_store.begin()) {
    Serial.println("LittleFS mount failed");
  }
  if (!g_config_store.begin()) {
    Serial.println("Config store initialization failed");
  }
  g_history_store.setRetentionRows(g_config_store.current().history_retention_rows);

  connectWifi();
  setupTime();
  g_dashboard.begin();

  Serial.println("CO2 monitor ready");
}

void loop() {
  g_sensor.loop();
  g_dashboard.handleClient();

  const unsigned long now_ms = millis();

  if (g_first_sample_pending) {
    g_last_sample_ms = now_ms;
    g_last_alert_check_ms = now_ms;
    g_first_sample_pending = false;
    return;
  }

  const RuntimeConfig& runtime_config = g_config_store.current();

  if ((now_ms - g_last_alert_check_ms) >= kAlertCheckIntervalMs) {
    g_last_alert_check_ms = now_ms;
    g_notifier.maybeSendAlert(g_sensor.latestReading());
  }

  if ((now_ms - g_last_sample_ms) < runtime_config.save_interval_seconds * 1000UL) {
    return;
  }

  g_last_sample_ms = now_ms;
  g_history_store.setRetentionRows(runtime_config.history_retention_rows);
  const SensorReading reading = g_sensor.latestReading();
  g_history_store.appendSample(reading, runtime_config.store_temperature);
}
