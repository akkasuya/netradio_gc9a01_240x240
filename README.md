# NetRadio (ESP32-S3 + SSD1306 + MAX98357)

Internet radio on an ESP32-S3 with a 128x64 I2C SSD1306 OLED and a MAX98357 I2S amplifier.

Shows time, station name, playback state, IP address / mDNS name, volume, and a 16-band spectrum visualizer.

> Branch `oled-ssd1306`: this variant uses an SSD1306 OLED instead of the 240x240 GC9A01 round display. See `main` for the GC9A01 build.

## Hardware

| Part | Pin |
| --- | --- |
| SSD1306 SDA | GPIO12 |
| SSD1306 SCL | GPIO13 |
| SSD1306 VCC | 3.3V |
| SSD1306 GND | GND |
| MAX98357 LRC | GPIO1 |
| MAX98357 BCLK | GPIO2 |
| MAX98357 DIN | GPIO3 |

I2C address: `0x3C` (change `OLED_ADDR` in `src/config.h` if your module uses `0x3D`).
Display setup: `src/lgfx_config.h`.

## Screen layout

| Row | Content |
| --- | --- |
| top | time (large), volume `Vnn` at the top right |
| middle | station name |
| below | IP address / `netradio.local` (alternating), or the playback state when not playing |
| bottom | 16-band spectrum visualizer |

## Setup

1. Copy `src/secrets.example.h` to `src/secrets.h` and fill in your WiFi SSID and password.
2. Build and flash:

```
pio run -t upload
pio device monitor
```

The boot log prints an I2C scan and the detected OLED address.

## Web interface

The device is reachable at `http://<device-ip>/` (shown on the display) or `http://netradio.local/` via mDNS.

Controls:

- play / pause / stop
- pick a station from the list
- adjust the volume (0–21)

API:

| Endpoint | Method | Description |
| --- | --- | --- |
| `/` | GET | control page |
| `/api/stations` | GET | station names as JSON |
| `/api/status` | GET | IP, station, volume, state, time |
| `/api/station?i=N` | POST | select station `N` |
| `/api/volume?v=N` | POST | set volume (0–21) |
| `/api/play` | POST | resume (restarts the stream if stopped) |
| `/api/pause` | POST | pause / resume toggle |
| `/api/stop` | POST | stop playback (no auto-retry) |

## Configuration

- Stations: `STATIONS[]` in `src/config.h`
- Timezone: `TZ_INFO` in `src/config.h` (default `ICT-7`)
- mDNS host name: `MDNS_HOST` in `src/config.h` (default `netradio`)
- OLED address / pins: `OLED_ADDR`, `OLED_SDA`, `OLED_SCL` in `src/config.h`
- Screen rotation: `tft.setRotation(2)` in `src/ui.cpp`
- Refresh interval: `UI_FRAME_MS` in `src/config.h`
- I2C scan at boot: `OLED_I2C_SCAN` in `src/config.h`

## Notes

- PSRAM must be enabled (the audio library allocates its stream buffer there):
  `board_build.arduino.memory_type = qio_qspi` and `board_build.psram_type = qspi` in `platformio.ini`.
- Only ASCII text is rendered.
- Station URLs go stale over time. If a station stays in `BUFFERING` or `STREAM ERROR`, replace its URL in `src/config.h`.
- `tools/lcdtest` is the GC9A01 diagnostic sketch from `main`; it does not apply to this branch.
