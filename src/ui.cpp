#include "ui.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>
#include "lgfx_config.h"

static LGFX tft;
static LGFX_Sprite canvas(&tft);
static LGFX_Sprite textLayer(&tft);

static const int CX = 120;
static const int CY = 120;

// Text is alpha-blended over the background; 0..255
static const int TEXT_ALPHA = 230;

struct Box { int x, y, w, h; };
static const Box TEXT_BOXES[] = {
    {40, 8, 160, 66},    // time
    {36, 66, 168, 22},   // status
    {22, 86, 196, 34},   // station
    {56, 170, 128, 20},  // mDNS host name
    {56, 188, 128, 20},  // state
};

// Circular visualizer on the back layer
static const int BAR_COUNT = 32;
static const int BAR_R0 = 34;
static const int BAR_R1 = 100;

// Volume gauge: arc around the dial, gap at the top (sweep 270 deg)
static const int GAUGE_R_OUT = 112;
static const int GAUGE_R_IN  = 104;
static const float GAUGE_START = 45.0f;   // degrees clockwise from top
static const float GAUGE_SWEEP = 270.0f;

// Layout (y positions on the 240px circle)
static const int Y_TIME    = 14;
static const int Y_STATUS  = 68;
static const int Y_STATION = 88;
static const int Y_VOLTXT  = 172;
static const int Y_STATE   = 190;

static char timeText[8]    = "--:--";
static char statusText[32] = "";
static char stationText[32] = "";
static char stateText[24]  = "";
static int volNow = 0;
static int volMax = 1;
static float bars[SPEC_BANDS] = {0};

static void drawCentered(const char* s, int y, const lgfx::IFont* font, uint16_t color) {
    textLayer.setFont(font);
    textLayer.setTextColor(color, TFT_BLACK);
    textLayer.setTextDatum(top_center);
    textLayer.drawString(s, CX, y);
}

static void blendText() {
    for (const Box& b : TEXT_BOXES) {
        for (int y = b.y; y < b.y + b.h; y++) {
            for (int x = b.x; x < b.x + b.w; x++) {
                uint16_t fg = textLayer.readPixel(x, y);
                if (fg == 0) continue;
                uint16_t bg = canvas.readPixel(x, y);
                int fr = ((fg >> 11) & 31) << 3, fgG = ((fg >> 5) & 63) << 2, fb = (fg & 31) << 3;
                int br = ((bg >> 11) & 31) << 3, bgG = ((bg >> 5) & 63) << 2, bb = (bg & 31) << 3;
                int r = (fr * TEXT_ALPHA + br * (255 - TEXT_ALPHA)) / 255;
                int g = (fgG * TEXT_ALPHA + bgG * (255 - TEXT_ALPHA)) / 255;
                int bl = (fb * TEXT_ALPHA + bb * (255 - TEXT_ALPHA)) / 255;
                canvas.drawPixel(x, y, tft.color565(r, g, bl));
            }
        }
    }
}

static int polarX(float deg, int r) {
    return CX + (int)lroundf(r * sinf(deg * 3.14159265f / 180.0f));
}

static int polarY(float deg, int r) {
    return CY - (int)lroundf(r * cosf(deg * 3.14159265f / 180.0f));
}

static void arcSegment(float a0, float a1, int rIn, int rOut, uint16_t color) {
    for (float a = a0; a < a1; a += 1.0f) {
        float b = fminf(a + 1.0f, a1);
        int xo1 = polarX(a, rOut), yo1 = polarY(a, rOut);
        int xi1 = polarX(a, rIn),  yi1 = polarY(a, rIn);
        int xo2 = polarX(b, rOut), yo2 = polarY(b, rOut);
        int xi2 = polarX(b, rIn),  yi2 = polarY(b, rIn);
        canvas.fillTriangle(xo1, yo1, xi1, yi1, xo2, yo2, color);
        canvas.fillTriangle(xi1, yi1, xi2, yi2, xo2, yo2, color);
    }
}

// green -> yellow -> red along the sweep
static uint16_t gaugeColor(float t) {
    uint8_t r, g;
    if (t < 0.5f) {
        r = (uint8_t)(510.0f * t);
        g = 255;
    } else {
        r = 255;
        g = (uint8_t)(255.0f * (1.0f - (t - 0.5f) * 2.0f));
    }
    return canvas.color565(r, g, 0);
}

static float barRotation = 0.0f;
static const float BAR_SPIN_DEG_PER_MS = 0.008f;   // about 8 degrees per second

static void drawBars() {
    const float step = 360.0f / BAR_COUNT;
    for (int i = 0; i < BAR_COUNT; i++) {
        int band = i * SPEC_BANDS / BAR_COUNT;
        float level = bars[band];
        int r1 = BAR_R0 + (int)(level * (BAR_R1 - BAR_R0));
        if (r1 <= BAR_R0) continue;
        float center = i * step + barRotation;
        uint8_t r = (uint8_t)(255 * band / (SPEC_BANDS - 1));
        uint8_t g = (uint8_t)(255 * (SPEC_BANDS - 1 - band) / (SPEC_BANDS - 1));
        arcSegment(center - step * 0.35f, center + step * 0.35f, BAR_R0, r1, canvas.color565(r, g, 255));
    }
}

void ui_init() {
    tft.init();
    tft.setRotation(0);
    canvas.setPsram(true);
    canvas.createSprite(240, 240);
    textLayer.setPsram(true);
    textLayer.createSprite(240, 240);
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

void ui_reinit() {
    tft.init();
    tft.setRotation(0);
    canvas.fillScreen(TFT_BLACK);
    canvas.pushSprite(0, 0);
}

void ui_render() {
    static uint32_t lastSpin = millis();
    uint32_t now = millis();
    barRotation = fmodf(barRotation + (now - lastSpin) * BAR_SPIN_DEG_PER_MS, 360.0f);
    lastSpin = now;

    canvas.fillScreen(TFT_BLACK);

    drawBars();

    canvas.drawCircle(CX, CY, 118, TFT_DARKGREY);
    canvas.drawCircle(CX, CY, 119, TFT_DARKGREY);

    arcSegment(GAUGE_START, GAUGE_START + GAUGE_SWEEP, GAUGE_R_IN, GAUGE_R_OUT, canvas.color565(40, 40, 40));
    float frac = volMax > 0 ? (float)volNow / volMax : 0.0f;
    float end = GAUGE_START + GAUGE_SWEEP * frac;
    for (float a = GAUGE_START; a < end; a += 1.0f) {
        float t = (a - GAUGE_START) / GAUGE_SWEEP;
        arcSegment(a, fminf(a + 1.0f, end), GAUGE_R_IN, GAUGE_R_OUT, gaugeColor(t));
    }

    textLayer.fillScreen(TFT_BLACK);
    textLayer.setTextSize(0.85f);
    drawCentered(timeText, Y_TIME + 6, &fonts::Font7, TFT_WHITE);
    textLayer.setTextSize(1.0f);

    drawCentered(statusText, Y_STATUS, &fonts::Font2, TFT_CYAN);
    drawCentered(stationText, Y_STATION, &fonts::Font4, TFT_YELLOW);

    drawCentered(MDNS_HOST ".local", Y_VOLTXT, &fonts::Font2, TFT_WHITE);
    drawCentered(stateText, Y_STATE, &fonts::Font2, TFT_ORANGE);

    blendText();
    canvas.pushSprite(0, 0);
}
