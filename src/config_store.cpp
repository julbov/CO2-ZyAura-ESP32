#include "config_store.h"

#include "app_config.h"

bool ConfigStore::begin() {
  if (!prefs_.begin(kNamespace, false)) {
    return false;
  }

  loadFromPrefs();

  String error;
  if (!validateAndNormalize(config_, error)) {
    config_ = RuntimeConfig{};
    if (!validateAndNormalize(config_, error)) {
      return false;
    }
  }
  saveToPrefs();
  return true;
}

const RuntimeConfig& ConfigStore::current() const {
  return config_;
}

void ConfigStore::writeJson(JsonObject target) const {
  target["device_name"] = config_.device_name;
  target["save_interval_seconds"] = config_.save_interval_seconds;
  target["history_retention_rows"] = config_.history_retention_rows;
  target["co2_threshold_ppm"] = config_.co2_threshold_ppm;
  target["alert_step_ppm"] = config_.alert_step_ppm;
  target["telegram_cooldown_seconds"] = config_.telegram_cooldown_seconds;
  target["store_temperature"] = config_.store_temperature;
  target["telegram_enabled"] = config_.telegram_enabled;
  target["telegram_message_template"] = config_.telegram_message_template;
}

bool ConfigStore::updateFromJson(JsonVariantConst source, String& error) {
  RuntimeConfig candidate = config_;

  if (source["device_name"].is<const char*>()) {
    candidate.device_name = source["device_name"].as<const char*>();
  }
  if (source["save_interval_seconds"].is<unsigned long>()) {
    candidate.save_interval_seconds = source["save_interval_seconds"].as<unsigned long>();
  }
  if (source["history_retention_rows"].is<size_t>()) {
    candidate.history_retention_rows = source["history_retention_rows"].as<size_t>();
  }
  if (source["co2_threshold_ppm"].is<uint16_t>()) {
    candidate.co2_threshold_ppm = source["co2_threshold_ppm"].as<uint16_t>();
  }
  if (source["alert_step_ppm"].is<uint16_t>()) {
    candidate.alert_step_ppm = source["alert_step_ppm"].as<uint16_t>();
  }
  if (source["telegram_cooldown_seconds"].is<unsigned long>()) {
    candidate.telegram_cooldown_seconds = source["telegram_cooldown_seconds"].as<unsigned long>();
  }
  if (source["store_temperature"].is<bool>()) {
    candidate.store_temperature = source["store_temperature"].as<bool>();
  }
  if (source["telegram_enabled"].is<bool>()) {
    candidate.telegram_enabled = source["telegram_enabled"].as<bool>();
  }
  if (source["telegram_message_template"].is<const char*>()) {
    candidate.telegram_message_template = source["telegram_message_template"].as<const char*>();
  }

  if (!validateAndNormalize(candidate, error)) {
    return false;
  }

  config_ = candidate;
  saveToPrefs();
  return true;
}

void ConfigStore::loadFromPrefs() {
  config_.device_name = prefs_.getString("name", config_.device_name);
  config_.save_interval_seconds =
      prefs_.getULong("svInt", config_.save_interval_seconds);
  config_.history_retention_rows =
      prefs_.getULong("histRows", config_.history_retention_rows);
  config_.co2_threshold_ppm = prefs_.getUShort("co2Hi", config_.co2_threshold_ppm);
  config_.alert_step_ppm = prefs_.getUShort("co2Step", config_.alert_step_ppm);
  config_.telegram_cooldown_seconds =
      prefs_.getULong("tgCool", config_.telegram_cooldown_seconds);
  config_.store_temperature = prefs_.getBool("storeT", config_.store_temperature);
  config_.telegram_enabled = prefs_.getBool("tgOn", config_.telegram_enabled);
  config_.telegram_message_template =
      prefs_.getString("tgMsg", config_.telegram_message_template);
}

void ConfigStore::saveToPrefs() {
  prefs_.putString("name", config_.device_name);
  prefs_.putULong("svInt", config_.save_interval_seconds);
  prefs_.putULong("histRows", config_.history_retention_rows);
  prefs_.putUShort("co2Hi", config_.co2_threshold_ppm);
  prefs_.putUShort("co2Step", config_.alert_step_ppm);
  prefs_.putULong("tgCool", config_.telegram_cooldown_seconds);
  prefs_.putBool("storeT", config_.store_temperature);
  prefs_.putBool("tgOn", config_.telegram_enabled);
  prefs_.putString("tgMsg", config_.telegram_message_template);
}

bool ConfigStore::validateAndNormalize(RuntimeConfig& candidate, String& error) const {
  candidate.device_name.trim();
  if (candidate.device_name.isEmpty()) {
    error = "device_name must not be empty";
    return false;
  }
  if (candidate.save_interval_seconds < 5UL) {
    error = "save_interval_seconds must be at least 5";
    return false;
  }
  if (candidate.history_retention_rows < 10 ||
      candidate.history_retention_rows > app_config::MAX_HISTORY_LIMIT) {
    error = "history_retention_rows must be between 10 and " +
            String(app_config::MAX_HISTORY_LIMIT);
    return false;
  }
  if (candidate.co2_threshold_ppm < 400) {
    error = "co2_threshold_ppm must be at least 400";
    return false;
  }
  if (candidate.alert_step_ppm < 50) {
    error = "alert_step_ppm must be at least 50";
    return false;
  }
  if (candidate.telegram_cooldown_seconds < 10UL) {
    error = "telegram_cooldown_seconds must be at least 10";
    return false;
  }
  candidate.telegram_message_template.trim();
  if (candidate.telegram_enabled && candidate.telegram_message_template.isEmpty()) {
    error = "telegram_message_template must not be empty when telegram is enabled";
    return false;
  }
  return true;
}
