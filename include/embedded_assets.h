#pragma once

#include <pgmspace.h>

namespace embedded_assets {

inline const char kIndexHtml[] PROGMEM = R"=====(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>CO2 Monitor</title>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <main class="page">
    <header class="header">
      <h1 class="title">CO2 Monitor</h1>
    </header>

    <section class="panel metrics">
      <article class="metric">
        <div class="meta">CO2</div>
        <div class="reading" id="co2">--</div>
      </article>
      <article class="metric">
        <div class="meta">Temperature</div>
        <div class="reading" id="temp">--</div>
      </article>
    </section>

    <section class="panel">
      <div class="toolbar">
        <div class="button-group">
          <button class="button button-secondary is-active" data-range="hour" type="button">Hour</button>
          <button class="button button-secondary" data-range="day" type="button">Day</button>
          <button class="button button-secondary" data-range="week" type="button">Week</button>
        </div>
        <div class="button-group">
          <button class="button button-secondary is-active" id="toggle-temperature" type="button">Temperature On</button>
        </div>
      </div>
      <canvas id="chart" width="960" height="320"></canvas>
      <p class="status" id="status">Loading...</p>
    </section>

    <details class="settings-panel">
      <summary>Settings</summary>
      <p class="status">Note: settings take effect after restart.</p>
      <p class="status" id="settings-status">Loading settings...</p>
      <form id="settings-form" novalidate>
        <div class="settings-grid">
          <label class="field">
            <span class="meta">Save Interval (sec)</span>
            <input id="save_interval_seconds" name="save_interval_seconds" type="number" min="5" step="1" required>
          </label>
          <label class="field">
            <span class="meta">Overwrite History (rows)</span>
            <input id="history_retention_rows" name="history_retention_rows" type="number" min="10" max="4320" step="1" required>
          </label>
          <label class="field">
            <span class="meta">CO2 Threshold (ppm)</span>
            <input id="co2_threshold_ppm" name="co2_threshold_ppm" type="number" min="400" step="1" required>
          </label>
          <label class="field">
            <span class="meta">Alert Step (ppm)</span>
            <input id="alert_step_ppm" name="alert_step_ppm" type="number" min="50" step="1" required>
            <small class="meta">After the first alert, send another every time CO2 climbs this much above the last alerted level.</small>
          </label>
          <label class="field">
            <span class="meta">Telegram Cooldown (sec)</span>
            <input id="telegram_cooldown_seconds" name="telegram_cooldown_seconds" type="number" min="10" step="1" required>
          </label>
          <label class="field checkbox-field">
            <span class="meta">Store Temperature</span>
            <input id="store_temperature" name="store_temperature" type="checkbox">
          </label>
          <label class="field checkbox-field">
            <span class="meta">Telegram Enabled</span>
            <input id="telegram_enabled" name="telegram_enabled" type="checkbox">
          </label>
          <label class="field field-full">
            <span class="meta">Telegram Message</span>
            <textarea id="telegram_message_template" name="telegram_message_template" rows="5"></textarea>
          </label>
        </div>
        <div class="actions">
          <button class="button" id="save-settings" type="submit">Save Settings</button>
          <button class="button button-secondary" id="restart-device" type="button">Restart Device</button>
          <button class="button button-secondary" id="clear-history" type="button">Clear History</button>
        </div>
      </form>
    </details>
  </main>
  <script src="/app.js"></script>
</body>
</html>
)=====";

inline const char kStyleCss[] PROGMEM = R"=====(:root {
  --bg: #FBE8CE;
  --panel: #fffdf9;
  --ink: #1f2a2e;
  --muted: #6b7280;
  --line: #ddd6ca;
  --accent: #A2CB8B;
  --accent-soft: #ece6db;
}

body {
  margin: 0;
  font: 16px/1.4 "Segoe UI", sans-serif;
  background: var(--bg);
  color: var(--ink);
}

.page {
  max-width: 980px;
  margin: 0 auto;
  padding: 24px;
}

.page > * + * {
  margin-top: 20px;
}

.header {
  margin-bottom: 8px;
}

.title {
  margin: 0;
  font-size: 28px;
  font-weight: 700;
}

.panel {
  background: var(--panel);
  border-radius: 14px;
  padding: 16px;
}

.metrics {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  gap: 12px;
}

.metric {
  min-height: 88px;
}

.meta {
  color: var(--muted);
  font-size: 13px;
  margin-bottom: 8px;
}

.reading {
  font-size: 32px;
  font-weight: 700;
}

canvas {
  width: 100%;
  height: 320px;
  display: block;
}

.toolbar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 12px;
  margin-bottom: 12px;
  flex-wrap: wrap;
}

.button-group {
  display: inline-flex;
  gap: 6px;
  flex-wrap: wrap;
}

.status {
  color: var(--muted);
  margin-top: 8px;
  font-size: 13px;
}

.settings-panel {
  padding: 12px 0 0;
}

.settings-panel summary {
  font-size: 18px;
  font-weight: 700;
  cursor: pointer;
}

.settings-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
  gap: 12px;
  margin-top: 12px;
}

.field {
  display: grid;
  gap: 6px;
}

.field-full {
  grid-column: 1 / -1;
}

.checkbox-field input {
  width: 18px;
  height: 18px;
  align-content: start;
}

.actions {
  margin-top: 18px;
}

.button {
  border: 1px solid var(--accent);
  border-radius: 10px;
  background: var(--accent);
  color: #fff;
  padding: 8px 12px;
  font: inherit;
  font-weight: 500;
  cursor: pointer;
}

.button-secondary {
  background: var(--accent-soft);
  border-color: var(--line);
  color: var(--ink);
}

.button-secondary.is-active {
  background: var(--accent);
  border-color: var(--accent);
  color: #fff;
}
)=====";

inline const char kAppJs[] PROGMEM = R"=====(const chartRanges = {
  hour: { label: "Hour", limit: 60 },
  day: { label: "Day", limit: 1440 },
  week: { label: "Week", limit: 10080 },
};
let activeRange = "hour";
let temperatureVisible = true;

async function fetchJson(url) {
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`HTTP ${response.status}`);
  }
  return response.json();
}

async function sendJson(url, method, payload) {
  const response = await fetch(url, {
    method,
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(payload),
  });
  const data = await response.json();
  if (!response.ok) {
    throw new Error(data.error || `HTTP ${response.status}`);
  }
  return data;
}

function setText(id, value, suffix = "") {
  document.getElementById(id).textContent = value == null ? "--" : `${value}${suffix}`;
}

function formatAge(updatedAt) {
  if (!updatedAt || updatedAt === "unknown") {
    return "unknown age";
  }

  const updatedMs = Date.parse(updatedAt);
  if (Number.isNaN(updatedMs)) {
    return "unknown age";
  }

  const diffSeconds = Math.max(0, Math.round((Date.now() - updatedMs) / 1000));
  if (diffSeconds < 60) {
    return `${diffSeconds} sec ago`;
  }

  const diffMinutes = Math.round(diffSeconds / 60);
  return `${diffMinutes} min ago`;
}

function formatRangeTimestamp(ts) {
  if (!ts) {
    return "unknown";
  }
  return new Date(ts * 1000).toLocaleString([], {
    month: "short",
    day: "numeric",
    hour: "2-digit",
    minute: "2-digit",
  });
}

function formatXAxisLabel(ts, rangeKey) {
  const date = new Date(ts * 1000);
  if (rangeKey === "week") {
    return date.toLocaleDateString([], { weekday: "short" });
  }
  if (rangeKey === "day") {
    return date.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
  }
  return date.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
}

function updateTemperatureToggle() {
  const button = document.getElementById("toggle-temperature");
  const storeTemperature = document.getElementById("store_temperature")?.checked ?? true;
  button.disabled = !storeTemperature;
  button.textContent = temperatureVisible ? "Temperature" : "Temperature";
  button.classList.toggle("is-active", temperatureVisible);
}

function populateSettingsForm(settings) {
  document.getElementById("save_interval_seconds").value = settings.save_interval_seconds ?? "";
  document.getElementById("history_retention_rows").value = settings.history_retention_rows ?? "";
  document.getElementById("co2_threshold_ppm").value = settings.co2_threshold_ppm ?? "";
  document.getElementById("alert_step_ppm").value = settings.alert_step_ppm ?? "";
  document.getElementById("telegram_cooldown_seconds").value = settings.telegram_cooldown_seconds ?? "";
  document.getElementById("store_temperature").checked = Boolean(settings.store_temperature);
  document.getElementById("telegram_enabled").checked = Boolean(settings.telegram_enabled);
  document.getElementById("telegram_message_template").value = settings.telegram_message_template || "";
  if (!settings.store_temperature) {
    temperatureVisible = false;
  }
  updateTemperatureToggle();
}

function settingsPayloadFromForm() {
  return {
    save_interval_seconds: Number(document.getElementById("save_interval_seconds").value),
    history_retention_rows: Number(document.getElementById("history_retention_rows").value),
    co2_threshold_ppm: Number(document.getElementById("co2_threshold_ppm").value),
    alert_step_ppm: Number(document.getElementById("alert_step_ppm").value),
    telegram_cooldown_seconds: Number(document.getElementById("telegram_cooldown_seconds").value),
    store_temperature: document.getElementById("store_temperature").checked,
    telegram_enabled: document.getElementById("telegram_enabled").checked,
    telegram_message_template: document.getElementById("telegram_message_template").value,
  };
}

async function loadSettings() {
  const settings = await fetchJson("/api/settings");
  populateSettingsForm(settings);
  document.getElementById("settings-status").textContent = "Settings loaded";
}

async function saveSettings(event) {
  event.preventDefault();
  const button = document.getElementById("save-settings");
  const status = document.getElementById("settings-status");
  button.disabled = true;
  status.textContent = "Saving...";

  try {
    const saved = await sendJson("/api/settings", "POST", settingsPayloadFromForm());
    populateSettingsForm(saved);
    status.textContent = "Saved to device";
  } catch (error) {
    status.textContent = error.message;
  } finally {
    button.disabled = false;
  }
}

function drawChart(points, rangeKey) {
  const canvas = document.getElementById("chart");
  const ctx = canvas.getContext("2d");
  const width = canvas.width;
  const height = canvas.height;
  const padLeft = 36;
  const padRight = 20;
  const padTop = 20;
  const padBottom = 44;

  ctx.clearRect(0, 0, width, height);
  ctx.fillStyle = "#fffdf8";
  ctx.fillRect(0, 0, width, height);

  if (!points.length) {
    ctx.fillStyle = "#66757f";
    ctx.font = "16px sans-serif";
    ctx.fillText("No history yet", 40, 40);
    return;
  }

  const values = points
    .map((point) => point.co2_ppm)
    .filter((value) => value != null);
  const temperatureValues = points
    .map((point) => point.temperature_c)
    .filter((value) => value != null);

  const min = Math.min(...values, 400);
  const max = Math.max(...values, 1600);
  const range = Math.max(100, max - min);
  const tempMin = temperatureValues.length ? Math.min(...temperatureValues) : 0;
  const tempMax = temperatureValues.length ? Math.max(...temperatureValues) : 0;
  const tempRange = Math.max(1, tempMax - tempMin);
  const plotWidth = width - padLeft - padRight;
  const plotHeight = height - padTop - padBottom;

  ctx.strokeStyle = "#d9d3c8";
  ctx.lineWidth = 1;
  for (let i = 0; i < 5; i += 1) {
    const y = padTop + (plotHeight * i) / 4;
    ctx.beginPath();
    ctx.moveTo(padLeft, y);
    ctx.lineTo(width - padRight, y);
    ctx.stroke();
  }

  ctx.strokeStyle = "#0f766e";
  ctx.lineWidth = 3;
  ctx.beginPath();
  points.forEach((point, index) => {
    const x = padLeft + (plotWidth * index) / Math.max(1, points.length - 1);
    const y = height - padBottom - ((point.co2_ppm - min) / range) * plotHeight;
    if (index === 0) {
      ctx.moveTo(x, y);
    } else {
      ctx.lineTo(x, y);
    }
  });
  ctx.stroke();

  ctx.fillStyle = "#66757f";
  ctx.font = "13px sans-serif";
  ctx.fillText(`${Math.round(max)} ppm`, 8, padTop + 4);
  ctx.fillText(`${Math.round(min)} ppm`, 8, height - padBottom + 4);

  if (temperatureVisible && temperatureValues.length) {
    ctx.strokeStyle = "#b45309";
    ctx.lineWidth = 2;
    ctx.beginPath();
    points.forEach((point, index) => {
      if (point.temperature_c == null) {
        return;
      }
      const x = padLeft + (plotWidth * index) / Math.max(1, points.length - 1);
      const y = height - padBottom - ((point.temperature_c - tempMin) / tempRange) * plotHeight;
      if (index === 0 || points[index - 1].temperature_c == null) {
        ctx.moveTo(x, y);
      } else {
        ctx.lineTo(x, y);
      }
    });
    ctx.stroke();

    ctx.fillStyle = "#b45309";
    ctx.textAlign = "right";
    ctx.fillText(`${tempMax.toFixed(1)} C`, width - 6, padTop + 4);
    ctx.fillText(`${tempMin.toFixed(1)} C`, width - 6, height - padBottom + 4);
    ctx.textAlign = "start";
  }

  const tickCount = rangeKey === "week" ? 7 : 5;
  const labelY = height - 14;
  ctx.textAlign = "center";
  for (let i = 0; i < tickCount; i += 1) {
    const pointIndex = Math.round((i * (points.length - 1)) / Math.max(1, tickCount - 1));
    const point = points[pointIndex];
    const x = padLeft + (plotWidth * pointIndex) / Math.max(1, points.length - 1);
    ctx.fillText(formatXAxisLabel(point.ts, rangeKey), x, labelY);
  }
  ctx.textAlign = "start";
}

function setActiveRange(rangeKey) {
  activeRange = rangeKey;
  document.querySelectorAll("[data-range]").forEach((button) => {
    button.classList.toggle("is-active", button.dataset.range === rangeKey);
  });
}

async function refresh() {
  const range = chartRanges[activeRange];
  const [latest, history] = await Promise.all([
    fetchJson("/api/latest"),
    fetchJson(`/api/history?limit=${range.limit}&max_points=360`),
  ]);

  setText("co2", latest.co2_ppm == null ? null : Math.round(latest.co2_ppm), " ppm");
  setText("temp", latest.temperature_c == null ? null : latest.temperature_c.toFixed(1), " C");

  const samples = history.samples || [];
  drawChart(samples, activeRange);
  const updatedLabel = latest.updated_at || "unknown";
  const ageLabel = formatAge(updatedLabel);
  const firstTs = samples.length ? formatRangeTimestamp(samples[0].ts) : "unknown";
  const lastTs = samples.length ? formatRangeTimestamp(samples[samples.length - 1].ts) : "unknown";
  document.getElementById("status").textContent =
    `Updated ${updatedLabel} (${ageLabel}) | ${range.label} view | ${samples.length} plotted samples | ${firstTs} to ${lastTs}`;
}

refresh().catch((error) => {
  document.getElementById("status").textContent = error.message;
});
loadSettings().catch((error) => {
  document.getElementById("settings-status").textContent = error.message;
});

setInterval(() => {
  refresh().catch(() => {});
}, 15000);

document.getElementById("settings-form").addEventListener("submit", saveSettings);
document.getElementById("restart-device").addEventListener("click", async () => {
  const button = document.getElementById("restart-device");
  const status = document.getElementById("settings-status");
  if (!confirm("Restart the device now? Active connections will drop for ~5 seconds.")) {
    return;
  }
  button.disabled = true;
  status.textContent = "Restarting...";
  try {
    await fetch("/api/restart", { method: "POST" });
  } catch (_) {
    // Connection drops are expected during restart.
  }
  setTimeout(() => window.location.reload(), 5000);
});
document.getElementById("clear-history").addEventListener("click", async () => {
  const button = document.getElementById("clear-history");
  const status = document.getElementById("settings-status");
  if (!confirm("Delete all stored history? This cannot be undone.")) {
    return;
  }
  button.disabled = true;
  status.textContent = "Clearing history...";
  try {
    const response = await fetch("/api/history/clear", { method: "POST" });
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    status.textContent = "History cleared";
    refresh().catch(() => {});
  } catch (error) {
    status.textContent = error.message;
  } finally {
    button.disabled = false;
  }
});
document.querySelectorAll("[data-range]").forEach((button) => {
  button.addEventListener("click", () => {
    setActiveRange(button.dataset.range);
    refresh().catch((error) => {
      document.getElementById("status").textContent = error.message;
    });
  });
});
document.getElementById("toggle-temperature").addEventListener("click", () => {
  if (!document.getElementById("store_temperature").checked) {
    return;
  }
  temperatureVisible = !temperatureVisible;
  updateTemperatureToggle();
  refresh().catch((error) => {
    document.getElementById("status").textContent = error.message;
  });
});
document.getElementById("store_temperature").addEventListener("change", (event) => {
  if (!event.target.checked) {
    temperatureVisible = false;
  } else {
    temperatureVisible = true;
  }
  updateTemperatureToggle();
  refresh().catch((error) => {
    document.getElementById("status").textContent = error.message;
  });
});
updateTemperatureToggle();
)=====";

}  // namespace embedded_assets
