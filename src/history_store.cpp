#include "history_store.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <math.h>
#include <time.h>

namespace {

#pragma pack(push, 1)
struct PersistedSample {
  uint32_t ts;
  float co2_ppm;
  float temperature_c;  // NaN means no temperature
};
#pragma pack(pop)
static_assert(sizeof(PersistedSample) == 12, "PersistedSample must be 12 bytes on disk");

}  // namespace

HistoryStore::HistoryStore(size_t retention_rows) : retention_rows_(retention_rows) {
  samples_.reserve(retention_rows_);
}

bool HistoryStore::begin() {
  if (!LittleFS.begin(true)) {
    Serial.println("[history] LittleFS mount failed");
    return false;
  }
  loadFromFile();
  Serial.printf("[history] loaded %u persisted samples\n",
                static_cast<unsigned>(samples_.size()));
  return true;
}

void HistoryStore::loadFromFile() {
  File file = LittleFS.open(kHistoryPath, "r");
  if (!file) {
    return;
  }

  PersistedSample record;
  while (file.read(reinterpret_cast<uint8_t*>(&record), sizeof(record)) ==
         sizeof(record)) {
    StoredSample sample;
    sample.ts = record.ts;
    sample.co2_ppm = record.co2_ppm;
    sample.has_temperature = !isnan(record.temperature_c);
    sample.temperature_c = record.temperature_c;
    samples_.push_back(sample);
  }
  file.close();

  if (samples_.size() > retention_rows_) {
    enforceRetention();
    rewriteFile();
  }
}

void HistoryStore::appendSample(const SensorReading& reading, bool store_temperature) {
  const time_t now = time(nullptr);
  if (now <= 0 || !reading.has_co2) {
    return;
  }

  StoredSample sample;
  sample.ts = static_cast<uint32_t>(now);
  sample.co2_ppm = reading.co2_ppm;
  sample.has_temperature =
      store_temperature && reading.has_temperature && !isnan(reading.temperature_c);
  sample.temperature_c = sample.has_temperature ? reading.temperature_c : NAN;

  samples_.push_back(sample);

  if (samples_.size() > retention_rows_) {
    enforceRetention();
    rewriteFile();
  } else {
    persistAppend(sample);
  }
}

void HistoryStore::setRetentionRows(size_t retention_rows) {
  if (retention_rows == retention_rows_) {
    return;
  }
  const size_t prev_size = samples_.size();
  retention_rows_ = retention_rows;
  enforceRetention();
  if (samples_.size() != prev_size) {
    rewriteFile();
  }
}

void HistoryStore::clearAll() {
  samples_.clear();
  if (LittleFS.exists(kHistoryPath)) {
    LittleFS.remove(kHistoryPath);
  }
  Serial.println("[history] cleared");
}

void HistoryStore::enforceRetention() {
  if (samples_.size() <= retention_rows_) {
    return;
  }
  const size_t excess = samples_.size() - retention_rows_;
  samples_.erase(samples_.begin(), samples_.begin() + excess);
}

void HistoryStore::persistAppend(const StoredSample& sample) {
  File file = LittleFS.open(kHistoryPath, "a");
  if (!file) {
    return;
  }

  PersistedSample record;
  record.ts = sample.ts;
  record.co2_ppm = sample.co2_ppm;
  record.temperature_c = sample.has_temperature ? sample.temperature_c : NAN;
  file.write(reinterpret_cast<const uint8_t*>(&record), sizeof(record));
  file.close();
}

void HistoryStore::rewriteFile() {
  File file = LittleFS.open(kHistoryPath, "w");
  if (!file) {
    return;
  }

  for (const auto& sample : samples_) {
    PersistedSample record;
    record.ts = sample.ts;
    record.co2_ppm = sample.co2_ppm;
    record.temperature_c = sample.has_temperature ? sample.temperature_c : NAN;
    file.write(reinterpret_cast<const uint8_t*>(&record), sizeof(record));
  }
  file.close();
}
