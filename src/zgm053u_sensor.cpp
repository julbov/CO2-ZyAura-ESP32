#include "zgm053u_sensor.h"

#include <time.h>

Zgm053uSensor* Zgm053uSensor::instance_ = nullptr;

Zgm053uSensor::Zgm053uSensor(uint8_t clock_pin, uint8_t data_pin)
    : clock_pin_(clock_pin), data_pin_(data_pin) {}

void Zgm053uSensor::begin() {
  instance_ = this;
  pinMode(clock_pin_, INPUT_PULLUP);
  pinMode(data_pin_, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(clock_pin_), handleClockInterrupt, FALLING);
}

void Zgm053uSensor::loop() {
  uint8_t frame[kFrameSizeBytes];
  bool has_frame = false;

  portENTER_CRITICAL(&mux_);
  if (decoder_.bit_count == kFrameSizeBits) {
    for (size_t i = 0; i < kFrameSizeBytes; ++i) {
      frame[i] = decoder_.buffer[i];
      decoder_.buffer[i] = 0;
    }
    decoder_.bit_count = 0;
    has_frame = true;
  }
  portEXIT_CRITICAL(&mux_);

  if (has_frame) {
    processFrame(frame);
  }
}

SensorReading Zgm053uSensor::latestReading() const {
  SensorReading snapshot;
  portENTER_CRITICAL(&mux_);
  snapshot = latest_;
  portEXIT_CRITICAL(&mux_);
  return snapshot;
}

void IRAM_ATTR Zgm053uSensor::handleClockInterrupt() {
  if (instance_ != nullptr) {
    instance_->onClockFalling();
  }
}

void IRAM_ATTR Zgm053uSensor::onClockFalling() {
  portENTER_CRITICAL_ISR(&mux_);
  const unsigned long now = millis();
  if ((now - decoder_.last_edge_ms) > kFrameTimeoutMs) {
    decoder_.bit_count = 0;
    for (uint8_t i = 0; i < kFrameSizeBytes; ++i) {
      decoder_.buffer[i] = 0;
    }
  }
  decoder_.last_edge_ms = now;

  if (decoder_.bit_count >= kFrameSizeBits) {
    decoder_.bit_count = 0;
    portEXIT_CRITICAL_ISR(&mux_);
    return;
  }

  const uint8_t byte_index = decoder_.bit_count / 8;
  const bool data_bit = digitalRead(data_pin_);
  decoder_.buffer[byte_index] =
      static_cast<uint8_t>((decoder_.buffer[byte_index] << 1) | (data_bit ? 1 : 0));
  decoder_.bit_count++;
  portEXIT_CRITICAL_ISR(&mux_);
}

void Zgm053uSensor::processFrame(const uint8_t frame[kFrameSizeBytes]) {
  const uint8_t checksum =
      static_cast<uint8_t>(frame[kFrameType] + frame[kFrameHigh] + frame[kFrameLow]);
  if (frame[kFrameSum] != checksum || frame[kFrameEnd] != kFrameEndMarker) {
    return;
  }

  const uint16_t raw_value =
      static_cast<uint16_t>((frame[kFrameHigh] << 8) | frame[kFrameLow]);
  updateLatest(frame[kFrameType], raw_value);
}

void Zgm053uSensor::updateLatest(uint8_t type, uint16_t raw_value) {
  const float value = decodeMeasurement(type, raw_value);
  if (isnan(value)) {
    return;
  }

  portENTER_CRITICAL(&mux_);
  latest_.timestamp = time(nullptr);
  switch (type) {
    case kTypeTemperature:
      latest_.temperature_c = value;
      latest_.has_temperature = true;
      break;
    case kTypeCo2:
      latest_.co2_ppm = value;
      latest_.has_co2 = true;
      break;
    default:
      break;
  }
  portEXIT_CRITICAL(&mux_);
}

float Zgm053uSensor::decodeMeasurement(uint8_t type, uint16_t raw_value) const {
  switch (type) {
    case kTypeTemperature:
      return raw_value > 5970 ? NAN : (raw_value / 16.0f) - 273.15f;
    case kTypeCo2:
      return raw_value > 10000 ? NAN : static_cast<float>(raw_value);
    default:
      return NAN;
  }
}
