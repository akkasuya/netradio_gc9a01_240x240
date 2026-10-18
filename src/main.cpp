// =====================================================
//  NetRadio - ESP32-S3 internet radio
//  GC9A01 240x240 round display + MAX98357 I2S amp
//
//  Station and volume are controlled from the web page.
// =====================================================

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sntp.h>
#include "Audio.h"
#include "config.h"
#include "radio.h"
#include "spectrum.h"
#include "ui.h"
#include "web.h"

enum PlayState { ST_IDLE, ST_BUFFERING, ST_PLAYING, ST_ERROR };

Audio audio;
static int stationIdx = 0;
static int volume = VOL_DEFAULT;
static PlayState pstate = ST_IDLE;
static bool netReady = false;
static uint32_t lastWifiAttempt = 0;
static uint32_t lastRetry = 0;

static const char* stateText(PlayState s) {
    switch (s) {
        case ST_BUFFERING: return "BUFFERING";
        case ST_PLAYING:   return "PLAYING";
        case ST_ERROR:     return "STREAM ERROR";
        default:           return "STOPPED";
    }
}

static void startStation() {
    audio.stopSong();
    pstate = ST_BUFFERING;
    ui_station(STATIONS[stationIdx].name);
    ui_state(stateText(pstate));
    Serial.printf("[RADIO] %s -> %s\n", STATIONS[stationIdx].name, STATIONS[stationIdx].url);
    if (!audio.connecttohost(STATIONS[stationIdx].url)) {
        pstate = ST_ERROR;
        ui_state(stateText(pstate));
    }
}

static void selectStation(int idx) {
    stationIdx = idx;
    if (netReady) {
        startStation();
    } else {
        ui_station(STATIONS[stationIdx].name);
    }
}

int radioGetStation() { return stationIdx; }

void radioSetStation(int idx) { selectStation(idx); }

int radioGetVolume() { return volume; }

void radioSetVolume(int vol) {
    volume = vol;
    audio.setVolume(volume);
    ui_volume(volume, VOL_MAX);
}

const char* radioStateText() { return stateText(pstate); }

static void wifiBegin() {
    lastWifiAttempt = millis();
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    ui_status("CONNECTING WIFI...");
    Serial.printf("[WIFI] Connecting to %s\n", WIFI_SSID);
}

void setup() {
    Serial.begin(115200);

    ui_init();
    ui_station(STATIONS[stationIdx].name);
    ui_volume(volume, VOL_MAX);
    ui_state(stateText(ST_IDLE));

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DIN);
    audio.setVolume(volume);

    wifiBegin();
}

void loop() {
    audio.loop();
    web_loop();

    // ----- WiFi -----
    if (WiFi.status() == WL_CONNECTED) {
        if (!netReady) {
            netReady = true;
            Serial.printf("[WIFI] Connected, IP %s\n", WiFi.localIP().toString().c_str());
            ui_status(WiFi.localIP().toString().c_str());
            configTzTime(TZ_INFO, NTP_SERVER);
            struct tm t;
            for (int i = 0; i < 20 && !getLocalTime(&t, 500); i++) {}
            esp_sntp_stop();
            web_begin();
            startStation();
        }
    } else {
        if (netReady) {
            netReady = false;
            audio.stopSong();
            pstate = ST_IDLE;
            ui_status("WIFI LOST");
            ui_state("NO WIFI");
        }
        if (millis() - lastWifiAttempt > 15000) wifiBegin();
    }

    // ----- Stream state -----
    if (pstate == ST_BUFFERING && audio.isRunning()) {
        pstate = ST_PLAYING;
        ui_state(stateText(pstate));
    }
    if (pstate == ST_PLAYING && !audio.isRunning()) {
        pstate = ST_ERROR;
        ui_state(stateText(pstate));
    }
    if (pstate == ST_ERROR && netReady && millis() - lastRetry > 5000) {
        lastRetry = millis();
        Serial.println("[RADIO] Retrying stream");
        startStation();
    }

    // ----- Clock -----
    static uint32_t lastClock = 0;
    if (millis() - lastClock > 500) {
        lastClock = millis();
        struct tm t;
        if (getLocalTime(&t, 0)) {
            char buf[8];
            strftime(buf, sizeof(buf), "%H:%M", &t);
            ui_time(buf);
        } else {
            ui_time("--:--");
        }
    }

    // ----- Visualizer -----
    static uint32_t lastViz = 0;
    static float disp[SPEC_BANDS] = {0};
    if (millis() - lastViz > 100) {
        lastViz = millis();
        float target[SPEC_BANDS] = {0};
        bool fresh = (pstate == ST_PLAYING) && spectrum_levels(target, audio.getSampleRate());
        for (int b = 0; b < SPEC_BANDS; b++) {
            float t = fresh ? target[b] : 0.0f;
            disp[b] = t > disp[b] ? t : disp[b] * 0.85f;
        }
        ui_bars(disp);
        ui_render();
    }
}
