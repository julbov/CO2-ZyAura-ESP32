#pragma once

#include <stdint.h>

#include "secrets.h"

namespace app_config {

constexpr uint8_t SENSOR_CLOCK_PIN = 3;
constexpr uint8_t SENSOR_DATA_PIN = 4;
constexpr size_t DEFAULT_HISTORY_LIMIT = 288;
constexpr size_t MAX_HISTORY_LIMIT = 4320;
constexpr size_t DEFAULT_HISTORY_MAX_POINTS = 360;
constexpr size_t MAX_HISTORY_MAX_POINTS = 2000;
constexpr size_t CONFIG_JSON_CAPACITY = 1024;

}  // namespace app_config
