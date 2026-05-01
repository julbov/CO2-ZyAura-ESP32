#pragma once

#include <WebServer.h>

#include "config_store.h"
#include "history_store.h"
#include "models.h"

class DashboardServer {
 public:
  DashboardServer(HistoryStore& history_store, ConfigStore& config_store,
                  SensorReading (*reading_provider)());

  void begin();
  void handleClient();

 private:
  void registerRoutes();
  void handleLatest();
  void handleHistory();
  void handleClearHistory();
  void handleGetSettings();
  void handlePostSettings();
  void handleRestart();

  HistoryStore& history_store_;
  ConfigStore& config_store_;
  SensorReading (*reading_provider_)();
  WebServer server_;
};
