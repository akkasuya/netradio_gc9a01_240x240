#pragma once
#include "secrets.h"

// ---- SSD1306 128x64 OLED (I2C) ----
#define OLED_SDA  12
#define OLED_SCL  13
#define OLED_ADDR 0x3C
#define OLED_W    128
#define OLED_H     64

// ---- MAX98357 I2S amplifier (from pin.txt) ----
#define I2S_LRC   1
#define I2S_BCLK  2
#define I2S_DIN   3

// ---- Time ----
#define NTP_SERVER "pool.ntp.org"
#define TZ_INFO    "ICT-7"

// ---- mDNS ----
#define MDNS_HOST "netradio"   // reachable as netradio.local

// ---- Display refresh ----
#define OLED_I2C_SCAN             1   // scan the I2C bus at boot and log devices
#define UI_FRAME_MS             120   // OLED is 1-bit, redraw is cheap
#define DISPLAY_REINIT_MS         0UL // re-init panel every N ms (0 = never)

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
