#pragma once
#include "spectrum.h"

void ui_init();
void ui_time(const char* hhmm);
void ui_status(const char* text);
void ui_station(const char* name);
void ui_bars(const float* levels);
void ui_volume(int vol, int maxVol);
void ui_state(const char* text);
void ui_render();
void ui_reinit();