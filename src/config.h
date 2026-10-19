#pragma once
#include "secrets.h"

// ---- GC9A01 240x240 round display (from pin.txt) ----
#define LCD_SCLK 13
#define LCD_MOSI 12
#define LCD_RST  11
#define LCD_DC   10
#define LCD_CS    9
// BLK tied to 3.3V (always on)

// ---- MAX98357 I2S amplifier (from pin.txt) ----
#define I2S_LRC   1
#define I2S_BCLK  2
#define I2S_DIN   3

// ---- Time ----
#define NTP_SERVER "pool.ntp.org"
#define TZ_INFO    "ICT-7"

// ---- mDNS ----
#define MDNS_HOST "netradio"   // reachable as netradio.local

// ---- Display timing / thermal stability ----
// GC9A01 panels on long jumper wires lose sync when hot at high SPI clocks.
// Lower these if the screen starts showing dots or tearing after a long run.
#define LCD_SPI_WRITE_HZ   10000000
#define LCD_SPI_READ_HZ     8000000
#define UI_FRAME_MS             200   // full-screen refresh interval
#define DISPLAY_REINIT_MS    600000UL // re-init panel every 10 min (0 = never)

// ---- Audio ----
#define VOL_MAX     21
#define VOL_DEFAULT 12

struct Station {
    const char* name;
    const char* url;
};

static const Station STATIONS[] = {
    {"coolfh",          "https://coolism-web.cdn.byteark.com/;stream/1"},
    {"greenwave",       "https://atimeonline3.smartclick.co.th/green"},
    {"efm",             "https://atimeonline3.smartclick.co.th/efm"},
    {"looktungnetwork", "https://cdn-th2.livestreaming.in.th/shoutcast/8200"},
    {"munforward",      "https://cdn-th2.livestreaming.in.th/shoutcast/8730"},
    {"great93",         "https://radio.vpsthai.net/proxy/qndrbfic/stream2"},
    {"yesterdayradio",  "http://radio4.thzhost.com:4122/;"},
    {"newsfamily",      "https://media.onair.one:8110/106"},
    {"xfm",             "https://radio12.plathong.net/7360/;stream.mp3"},
    {"hotwave",         "https://atimeonline3.smartclick.co.th/hotwave"},
    {"lovefm",          "http://radio10.plathong.net:7296/;"},
    {"fat_chiangrai",   "https://hp1.hostpleng.cloud/8272/stream"},
    {"love_chiangmai",  "https://radio11.plathong.net/7142/;stream.mp3"},
};
static const int STATION_COUNT = sizeof(STATIONS) / sizeof(STATIONS[0]);
