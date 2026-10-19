#pragma once

int radioGetStation();
void radioSetStation(int idx);
int radioGetVolume();
void radioSetVolume(int vol);
const char* radioStateText();

void radioPlay();
void radioPause();
void radioStop();
