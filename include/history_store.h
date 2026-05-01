#pragma once

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "models.h"

class HistoryStore {
 public:
  struct StoredSample {
    uint32_t ts;
    float co2_ppm;
    float temperature_c;
    bool has_temperature;
  };

  explicit HistoryStore(size_t retention_rows);

  bool begin();
  void appendSample(const SensorReading& reading, bool store_temperature);
  void setRetentionRows(size_t retention_rows);
  void clearAll();
  const std::vector<StoredSample>& samples() const { return samples_; }

 private:
  static constexpr const char* kHistoryPath = "/history.bin";

  void enforceRetention();
  void loadFromFile();
  void persistAppend(const StoredSample& sample);
  void rewriteFile();

  size_t retention_rows_;
  std::vector<StoredSample> samples_;
};
