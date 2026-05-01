#pragma once

#include <ArduinoJson.h>
#include <Preferences.h>

#include "runtime_config.h"

class ConfigStore {
 public:
  bool begin();
  const RuntimeConfig& current() const;
  void writeJson(JsonObject target) const;
  bool updateFromJson(JsonVariantConst source, String& error);

 private:
  static constexpr const char* kNamespace = "co2cfg";

  void loadFromPrefs();
  void saveToPrefs();
  bool validateAndNormalize(RuntimeConfig& candidate, String& error) const;

  Preferences prefs_;
  RuntimeConfig config_;
};
