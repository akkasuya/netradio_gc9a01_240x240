#include "ui.h"
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <string.h>
#include "lgfx_config.h"

static LGFX tft;

// 128x64 layout
static const int W = OLED_W;
static const int H = OLED_H;

static const int Y_TIME    = 0;   // Font2 (16px), flush to the top
static const int Y_STATION = 34;  // Font2, middle
static const int Y_INFO    = 46;  // Font0, middle
static const int Y_BARS    = 50;  // top of the visualizer area
static const int H_BARS    = 14;

static char timeText[8]     = "--:--";
static char statusText[24]  = "";
static char stationText[32] = "";
static char stateText[24]   = "";
static int volNow = 0;
static int volMax = 1;
static float bars[SPEC_BANDS] = {0};

static void drawTopCentered(const char* s, int y, const lgfx::IFont* font) {
    tft.setFont(font);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(top_center);
    tft.drawString(s, W / 2, y);
}

static void drawCentered(const char* s, int y, const lgfx::IFont* font) {
    tft.setFont(font);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(middle_center);
    tft.drawString(s, W / 2, y);
}

#if OLED_I2C_SCAN
// One-shot bus scan before LovyanGFX takes the I2C port over.
static void i2cScan() {
    Wire.begin(OLED_SDA, OLED_SCL);
    int found = 0;
    for (uint8_t a = 0x08; a < 0x78; a++) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[I2C] device at 0x%02X\n", a);
            found++;
        }
    }
    if (!found) Serial.println("[I2C] no device found");
    Wire.end();
}
#endif

void ui_init() {
#if OLED_I2C_SCAN
    i2cScan();
#endif
    bool ok = tft.init();
    Serial.printf("[OLED] init %s (%dx%d, addr 0x%02X, SDA %d SCL %d)\n",
                  ok ? "ok" : "FAILED", tft.width(), tft.height(),
                  OLED_ADDR, OLED_SDA, OLED_SCL);
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);
}

void ui_reinit() {
    tft.init();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);
}

void ui_time(const char* hhmm) {
    strncpy(timeText, hhmm, sizeof(timeText) - 1);
}

void ui_status(const char* text) {
    strncpy(statusText, text, sizeof(statusText) - 1);
}

void ui_station(const char* name) {
    strncpy(stationText, name, sizeof(stationText) - 1);
}

void ui_bars(const float* levels) {
    memcpy(bars, levels, sizeof(bars));
}

void ui_volume(int vol, int maxVol) {
    volNow = vol;
    volMax = maxVol;
}

void ui_state(const char* text) {
    strncpy(stateText, text, sizeof(stateText) - 1);
}

void ui_render() {
    // Info line: playback state takes priority, otherwise alternate between
    // the IP address and the mDNS name.
    const char* info = statusText;
    if (stateText[0] && strcmp(stateText, "PLAYING") != 0) {
        info = stateText;
    } else if ((millis() / 4000) % 2) {
        info = MDNS_HOST ".local";
    }

    char volBuf[12];
    snprintf(volBuf, sizeof(volBuf), "V%d", volNow);

    tft.startWrite();
    tft.fillScreen(TFT_BLACK);

    drawTopCentered(timeText, Y_TIME, &fonts::Font2);
    drawCentered(stationText, Y_STATION, &fonts::Font2);
    drawCentered(info, Y_INFO, &fonts::Font0);

    tft.setFont(&fonts::Font0);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(top_right);
    tft.drawString(volBuf, W - 1, 0);

    const int step = W / SPEC_BANDS;
    const int barW = step - 1;
    const int bottom = Y_BARS + H_BARS;
    for (int b = 0; b < SPEC_BANDS; b++) {
        int h = (int)(bars[b] * H_BARS);
        if (h < 1) continue;
        if (h > H_BARS) h = H_BARS;
        tft.fillRect(b * step, bottom - h, barW, h, TFT_WHITE);
    }

    tft.endWrite();
}
