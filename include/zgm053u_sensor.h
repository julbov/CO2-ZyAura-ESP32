#pragma once

#include <Arduino.h>

#include "models.h"

class Zgm053uSensor {
 public:
  Zgm053uSensor(uint8_t clock_pin, uint8_t data_pin);

  void begin();
  void loop();
  SensorReading latestReading() const;

 private:
  static constexpr unsigned long kFrameTimeoutMs = 10;
  static constexpr uint8_t kFrameSizeBits = 40;
  static constexpr uint8_t kFrameSizeBytes = 5;
  static constexpr uint8_t kFrameType = 0;
  static constexpr uint8_t kFrameHigh = 1;
  static constexpr uint8_t kFrameLow = 2;
  static constexpr uint8_t kFrameSum = 3;
  static constexpr uint8_t kFrameEnd = 4;
  static constexpr uint8_t kFrameEndMarker = 0x0D;
  static constexpr uint8_t kTypeHumidity = 0x41;
  static constexpr uint8_t kTypeTemperature = 0x42;
  static constexpr uint8_t kTypeCo2 = 0x50;

  struct DecoderState {
    volatile uint8_t buffer[kFrameSizeBytes] = {0};
    volatile uint8_t bit_count = 0;
    volatile unsigned long last_edge_ms = 0;
  };

  static Zgm053uSensor* instance_;
  static void IRAM_ATTR handleClockInterrupt();

  void IRAM_ATTR onClockFalling();
  void processFrame(const uint8_t frame[kFrameSizeBytes]);
  void updateLatest(uint8_t type, uint16_t raw_value);
  float decodeMeasurement(uint8_t type, uint16_t raw_value) const;

  uint8_t clock_pin_;
  uint8_t data_pin_;
  mutable portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  DecoderState decoder_;
  SensorReading latest_;
};
