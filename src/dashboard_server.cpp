#include <Arduino.h>
#include "dashboard_server.h"

#include <ArduinoJson.h>
#include <WiFi.h>

#include "app_config.h"
#include "embedded_assets.h"
#include "time_utils.h"

DashboardServer::DashboardServer(HistoryStore& history_store, ConfigStore& config_store,
                                 SensorReading (*reading_provider)())
    : history_store_(history_store),
      config_store_(config_store),
      reading_provider_(reading_provider),
      server_(80) {}

void DashboardServer::begin() {
  registerRoutes();
  server_.begin();
}

void DashboardServer::handleClient() {
  server_.handleClient();
}

void DashboardServer::registerRoutes() {
  server_.on("/", HTTP_GET, [this]() {
    server_.sendHeader("Connection", "close");
    server_.sendHeader("Cache-Control", "public, max-age=300");
    server_.send_P(200, "text/html; charset=utf-8", embedded_assets::kIndexHtml);
  });

  server_.on("/style.css", HTTP_GET, [this]() {
    server_.sendHeader("Connection", "close");
    server_.sendHeader("Cache-Control", "public, max-age=86400");
    server_.send_P(200, "text/css; charset=utf-8", embedded_assets::kStyleCss);
  });

  server_.on("/app.js", HTTP_GET, [this]() {
    server_.sendHeader("Connection", "close");
    server_.sendHeader("Cache-Control", "public, max-age=86400");
    server_.send_P(200, "application/javascript; charset=utf-8", embedded_assets::kAppJs);
  });

  server_.on("/favicon.ico", HTTP_GET, [this]() {
    server_.sendHeader("Connection", "close");
    server_.sendHeader("Cache-Control", "public, max-age=86400");
    server_.send(204, "image/x-icon", "");
  });

  server_.on("/api/latest", HTTP_GET, [this]() { handleLatest(); });
  server_.on("/api/history", HTTP_GET, [this]() { handleHistory(); });
  server_.on("/api/settings", HTTP_GET, [this]() { handleGetSettings(); });
  server_.on("/api/settings", HTTP_POST, [this]() { handlePostSettings(); });
  server_.on("/api/restart", HTTP_POST, [this]() { handleRestart(); });
  server_.on("/api/history/clear", HTTP_POST, [this]() { handleClearHistory(); });

  server_.onNotFound([this]() {
    server_.sendHeader("Connection", "close");
    server_.send(404, "text/plain", "Not found");
  });
}

void DashboardServer::handleLatest() {
  const SensorReading reading = reading_provider_();
  const RuntimeConfig& runtime_config = config_store_.current();

  StaticJsonDocument<256> doc;
  if (reading.has_co2 && !isnan(reading.co2_ppm)) {
    doc["co2_ppm"] = reading.co2_ppm;
  }
  if (runtime_config.store_temperature && reading.has_temperature && !isnan(reading.temperature_c)) {
    doc["temperature_c"] = reading.temperature_c;
  }
  doc["updated_at"] = isoTime(reading.timestamp);
  doc["ip"] = WiFi.localIP().toString();

  String body;
  serializeJson(doc, body);
  server_.sendHeader("Connection", "close");
  server_.send(200, "application/json", body);
}

void DashboardServer::handleHistory() {
  size_t limit = app_config::DEFAULT_HISTORY_LIMIT;
  size_t max_points = app_config::DEFAULT_HISTORY_MAX_POINTS;
  if (server_.hasArg("limit")) {
    limit = constrain(server_.arg("limit").toInt(), 1, static_cast<int>(app_config::MAX_HISTORY_LIMIT));
  }
  if (server_.hasArg("max_points")) {
    max_points = constrain(server_.arg("max_points").toInt(), 10,
                           static_cast<int>(app_config::MAX_HISTORY_MAX_POINTS));
  }

  const auto& all = history_store_.samples();
  const size_t total = all.size();
  const size_t kept = total > limit ? limit : total;
  const size_t start = total - kept;
  const size_t stride =
      (kept == 0 || max_points == 0 || kept <= max_points) ? 1 : (kept + max_points - 1) / max_points;

  server_.sendHeader("Connection", "close");
  server_.sendHeader("Cache-Control", "no-store");
  server_.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server_.send(200, "application/json", "");
  server_.sendContent("{\"samples\":[");

  char buf[96];
  bool first = true;
  for (size_t i = 0; i < kept; ++i) {
    const bool is_last = (i + 1 == kept);
    if ((i % stride) != 0 && !is_last) {
      continue;
    }
    const HistoryStore::StoredSample& s = all[start + i];
    int n;
    if (s.has_temperature) {
      n = snprintf(buf, sizeof(buf),
                   "%s{\"ts\":%lu,\"co2_ppm\":%.0f,\"temperature_c\":%.2f}",
                   first ? "" : ",", static_cast<unsigned long>(s.ts), s.co2_ppm, s.temperature_c);
    } else {
      n = snprintf(buf, sizeof(buf), "%s{\"ts\":%lu,\"co2_ppm\":%.0f}",
                   first ? "" : ",", static_cast<unsigned long>(s.ts), s.co2_ppm);
    }
    if (n > 0) {
      server_.sendContent(buf);
    }
    first = false;
  }

  char tail[64];
  snprintf(tail, sizeof(tail), "],\"requested_limit\":%u,\"max_points\":%u}",
           static_cast<unsigned>(limit), static_cast<unsigned>(max_points));
  server_.sendContent(tail);
}

void DashboardServer::handleGetSettings() {
  StaticJsonDocument<app_config::CONFIG_JSON_CAPACITY> doc;
  JsonObject root = doc.to<JsonObject>();
  config_store_.writeJson(root);

  String body;
  serializeJson(doc, body);
  server_.sendHeader("Connection", "close");
  server_.send(200, "application/json", body);
}

void DashboardServer::handlePostSettings() {
  if (!server_.hasArg("plain")) {
    server_.sendHeader("Connection", "close");
    server_.send(400, "application/json", "{\"error\":\"missing JSON body\"}");
    return;
  }

  StaticJsonDocument<app_config::CONFIG_JSON_CAPACITY> doc;
  const DeserializationError json_error = deserializeJson(doc, server_.arg("plain"));
  if (json_error) {
    server_.sendHeader("Connection", "close");
    server_.send(400, "application/json", "{\"error\":\"invalid JSON body\"}");
    return;
  }

  String error;
  if (!config_store_.updateFromJson(doc.as<JsonVariantConst>(), error)) {
    StaticJsonDocument<256> response;
    response["error"] = error;
    String body;
    serializeJson(response, body);
    server_.sendHeader("Connection", "close");
    server_.send(400, "application/json", body);
    return;
  }

  StaticJsonDocument<app_config::CONFIG_JSON_CAPACITY> response;
  JsonObject root = response.to<JsonObject>();
  config_store_.writeJson(root);
  String body;
  serializeJson(response, body);
  server_.sendHeader("Connection", "close");
  server_.send(200, "application/json", body);
}

void DashboardServer::handleClearHistory() {
  history_store_.clearAll();
  server_.sendHeader("Connection", "close");
  server_.send(200, "application/json", "{\"cleared\":true}");
}

void DashboardServer::handleRestart() {
  server_.sendHeader("Connection", "close");
  server_.send(200, "application/json", "{\"restarting\":true}");
  delay(250);
  ESP.restart();
}

