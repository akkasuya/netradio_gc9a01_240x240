// Minimal GC9A01 test: solid colours, circle, text.
// Used to tell a panel/wiring fault apart from an application bug.
#include <Arduino.h>
#include <LovyanGFX.hpp>

#define LCD_SCLK 13
#define LCD_MOSI 12
#define LCD_RST  11
#define LCD_DC   10
#define LCD_CS    9

#ifndef SPI_HZ
#define SPI_HZ 10000000
#endif

#ifndef SPI_DMA
#define SPI_DMA SPI_DMA_CH_AUTO
#endif

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_GC9A01 _panel_instance;
    lgfx::Bus_SPI _bus_instance;

public:
    LGFX(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = SPI_HZ;
            cfg.freq_read = 8000000;
            cfg.spi_3wire = false;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA;
            cfg.pin_sclk = LCD_SCLK;
            cfg.pin_mosi = LCD_MOSI;
            cfg.pin_miso = -1;
            cfg.pin_dc = LCD_DC;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = LCD_CS;
            cfg.pin_rst = LCD_RST;
            cfg.pin_busy = -1;
            cfg.panel_width = 240;
            cfg.panel_height = 240;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true;
            cfg.rgb_order = true;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
};

static LGFX tft;

struct Step { uint16_t color; const char* name; };
static const Step STEPS[] = {
    {TFT_RED, "RED"}, {TFT_GREEN, "GREEN"}, {TFT_BLUE, "BLUE"},
    {TFT_WHITE, "WHITE"}, {TFT_BLACK, "BLACK"},
};

void setup() {
    Serial.begin(115200);
    delay(300);
    tft.init();
    tft.setRotation(0);
    Serial.printf("[LCDTEST] %dx%d @ %d Hz\n", tft.width(), tft.height(), SPI_HZ);
}

void loop() {
    for (const Step& s : STEPS) {
        tft.fillScreen(s.color);
        if (s.color != TFT_BLACK) {
            tft.setTextColor(TFT_BLACK);
            tft.setTextDatum(middle_center);
            tft.setFont(&fonts::Font4);
            tft.drawString(s.name, 120, 120);
        }
        Serial.printf("[LCDTEST] %s\n", s.name);
        delay(1500);
    }

    // Geometry check: outline, cross and text
    tft.fillScreen(TFT_BLACK);
    tft.drawCircle(120, 120, 100, TFT_WHITE);
    tft.drawLine(20, 120, 220, 120, TFT_RED);
    tft.drawLine(120, 20, 120, 220, TFT_GREEN);
    tft.setTextColor(TFT_YELLOW);
    tft.setTextDatum(middle_center);
    tft.setFont(&fonts::Font6);
    tft.drawString("TEST", 120, 120);
    Serial.println("[LCDTEST] geometry");
    delay(3000);
}
