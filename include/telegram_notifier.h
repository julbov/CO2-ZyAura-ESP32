#pragma once

#include <Arduino.h>

#include "config_store.h"
#include "models.h"

class TelegramNotifier {
 public:
  explicit TelegramNotifier(const ConfigStore& config_store);

  void maybeSendAlert(const SensorReading& reading);

 private:
  bool sendMessage(const String& message);
  String formatAlertMessage(const SensorReading& reading) const;

  unsigned long last_telegram_ms_ = 0;
  bool last_telegram_valid_ = false;
  float last_alert_co2_ = 0.0f;
  const ConfigStore& config_store_;
};
