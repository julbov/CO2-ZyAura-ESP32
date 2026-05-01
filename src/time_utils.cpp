#include "time_utils.h"

String isoTime(time_t timestamp) {
  if (timestamp <= 0) {
    return "unknown";
  }

  struct tm timeinfo;
  gmtime_r(&timestamp, &timeinfo);
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buffer);
}

bool timeIsReady() {
  return time(nullptr) > 1700000000;
}
