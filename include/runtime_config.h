#pragma once

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

#include "secrets.h"

struct RuntimeConfig {
  String device_name = config::DEVICE_NAME;
  unsigned long save_interval_seconds = config::SAVE_INTERVAL_SECONDS;
  size_t history_retention_rows = config::HISTORY_RETENTION_ROWS;
  uint16_t co2_threshold_ppm = config::CO2_THRESHOLD_PPM;
  uint16_t alert_step_ppm = config::ALERT_STEP_PPM;
  unsigned long telegram_cooldown_seconds = config::TELEGRAM_COOLDOWN_SECONDS;
  bool store_temperature = config::STORE_TEMPERATURE;
  bool telegram_enabled = config::TELEGRAM_ENABLED;
  String telegram_message_template = config::TELEGRAM_MESSAGE_TEMPLATE;
};
