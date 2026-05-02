# CO₂ Monitor

A simple CO₂ and temperature monitor for the ZyAura **ZGM053U** CO₂ meter.


## What it does

- Reads CO₂ and temperature directly from the ZGM053U over its clock/data interface.
- Hosts a small local dashboard with live values and a chart (hour / day / week).
- Sends a Telegram message when CO₂ goes over a configurable threshold, and another every time it climbs `Alert Step` ppm higher.
- History is kept on flash so the chart survives a reboot. Settings are persisted in NVS.

## Dashboard

<img width="1397" height="854" alt="image" src="https://github.com/user-attachments/assets/10eda425-2a54-428e-8aa7-8a0566e55999" />


## Hardware

- An **ESP32** dev board. I highly recommended the esp32 c3 supermini board, its very compact, and can even fit inside the sensor. It's what I used for this project.
- A **ZGM053U** CO₂ meter. The sensor gets its own 5 V USB power.
- Three jumper wires.

### Wiring
<img width="750" height="304" alt="sensor wiring" src="https://github.com/user-attachments/assets/f96637ee-699d-405c-a6de-069b64a201eb" />

[Image](https://esphome.io/components/sensor/zyaura)



| ZGM053U pin | Function | ESP32‑C3 |
|---|---|---|
| 1 | not connected | — |
| 2 | CLOCK | **GPIO4** |
| 3 | DATA  | **GPIO3** |
| 4 | GND   | **GND**  |

> If you get no readings, swap CLOCK and DATA.
<img width="625" height="475" alt="image" src="https://github.com/user-attachments/assets/65d5a71f-4186-4401-b518-1290f8aff9ee" />

## Setup

1. Install [PlatformIO](https://platformio.org/) (the VS Code extension is easiest).
2. Clone this repo.
3. Copy `include/secrets.example.h` to `include/secrets.h` and fill in:
   - Your **Wi‑Fi SSID and password** (the C3 only supports 2.4 GHz).
   - Optionally a **Telegram bot token** ([create one with `@BotFather`](https://core.telegram.org/bots#how-do-i-create-a-bot)).
   - And the telegram **chat ID** that should receive alerts.
4. Build and flash:
   ```bash
   pio run --target upload
   pio device monitor
   ```
5. Watch the serial monitor for the IP address, then open it in a browser. You can also check for devices on the local network and find the ip that way.

## Settings

All settings are editable from the dashboard, persisted to flash, and apply after **Restart**.

| Setting | Description | Default |
|---|---|---|
| Save Interval | How often a sample is recorded, in seconds | `60` |
| Overwrite History (rows) | Max rows saved | `1440` |
| CO2 Threshold (ppm) | First alert when CO₂ rises above this | `1200` |
| Alert Step (ppm) | Send another alert every time CO₂ climbs this much above the last alerted level | `200` |
| Telegram Cooldown (sec) |alerts can't fire more often than this | `1800` |
| Store Temperature | Record and display temperature | `on` |
## Telegram alerts

<img width="379" height="196" alt="telegram alerts" src="https://github.com/user-attachments/assets/b2aef867-a0a6-43b4-b1d8-8aea93bd79c9" />

### Note
- The first alert fires when CO₂ crosses the threshold.
- Each subsequent alert fires when CO₂ climbs another **Alert Step** ppm above the previous one. So a jump from 1200 to 3000 produces two messages: one at the crossing, one at the step.
- The "alerted" state resets when CO₂ drops back below the threshold, so the next crossing is a fresh first alert.
- The cooldown caps total frequency as a safety net.
