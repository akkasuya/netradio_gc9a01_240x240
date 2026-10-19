#pragma once
#include <LovyanGFX.hpp>
#include "config.h"

// SSD1306 128x64 on I2C (SCL/SDA from config.h)
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_SSD1306 _panel_instance;
    lgfx::Bus_I2C _bus_instance;

public:
    LGFX(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.i2c_port = 0;
            cfg.i2c_addr = OLED_ADDR;
            cfg.pin_sda = OLED_SDA;
            cfg.pin_scl = OLED_SCL;
            cfg.freq_write = 400000;
            cfg.freq_read = 400000;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.panel_width = OLED_W;
            cfg.panel_height = OLED_H;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.invert = false;
            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
};
