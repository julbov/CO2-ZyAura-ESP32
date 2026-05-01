#pragma once

#include <Arduino.h>
#include <time.h>

struct SensorReading {
  float co2_ppm = NAN;
  float temperature_c = NAN;
  time_t timestamp = 0;
  bool has_co2 = false;
  bool has_temperature = false;
};
