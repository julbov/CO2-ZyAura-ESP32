#include "telegram_notifier.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "secrets.h"

TelegramNotifier::TelegramNotifier(const ConfigStore& config_store)
    : config_store_(config_store) {}

void TelegramNotifier::maybeSendAlert(const SensorReading& reading) {
  if (!reading.has_co2 || isnan(reading.co2_ppm)) {
    return;
  }

  const RuntimeConfig& runtime_config = config_store_.current();
  if (!runtime_config.telegram_enabled) {
    return;
  }
  const unsigned long now_ms = millis();

  if (reading.co2_ppm < runtime_config.co2_threshold_ppm) {
    if (last_alert_co2_ > 0.0f) {
      Serial.println("[telegram] dropped below threshold, rearmed");
    }
    last_alert_co2_ = 0.0f;
    return;
  }

  const bool first_alert = last_alert_co2_ <= 0.0f;
  const bool stepped_up =
      !first_alert &&
      reading.co2_ppm >= last_alert_co2_ + static_cast<float>(runtime_config.alert_step_ppm);
  if (!first_alert && !stepped_up) {
    return;
  }

  if (last_telegram_valid_ &&
      (now_ms - last_telegram_ms_) < runtime_config.telegram_cooldown_seconds * 1000UL) {
    return;
  }

  Serial.printf("[telegram] sending alert co2=%.0f ppm (last=%.0f, step=%u)\n",
                reading.co2_ppm, last_alert_co2_, runtime_config.alert_step_ppm);
  if (sendMessage(formatAlertMessage(reading))) {
    last_telegram_ms_ = now_ms;
    last_telegram_valid_ = true;
    last_alert_co2_ = reading.co2_ppm;
    Serial.println("[telegram] alert sent");
  } else {
    Serial.println("[telegram] send failed");
  }
}

bool TelegramNotifier::sendMessage(const String& message) {
  if (String(config::TELEGRAM_BOT_TOKEN).isEmpty() || String(config::TELEGRAM_CHAT_ID).isEmpty()) {
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  const String url =
      "https://api.telegram.org/bot" + String(config::TELEGRAM_BOT_TOKEN) + "/sendMessage";
  if (!http.begin(client, url)) {
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  StaticJsonDocument<512> payload_doc;
  payload_doc["chat_id"] = config::TELEGRAM_CHAT_ID;
  payload_doc["text"] = message;

  String payload;
  serializeJson(payload_doc, payload);
  const int code = http.POST(payload);
  http.end();
  return code > 0 && code < 300;
}

String TelegramNotifier::formatAlertMessage(const SensorReading& reading) const {
  const RuntimeConfig& runtime_config = config_store_.current();
  String message = runtime_config.telegram_message_template;
  message.replace("{co2_ppm}", String(reading.co2_ppm, 0));
  message.replace("{temperature_c}",
                  isnan(reading.temperature_c) ? String("n/a") : String(reading.temperature_c, 1));
  return message;
}
