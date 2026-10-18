#pragma once
#include <stdint.h>

#define SPEC_N     512
#define SPEC_BANDS 16

// Fills 'levels' (0.0..1.0) for each band from the latest PCM window.
// Returns false when no new window is ready.
bool spectrum_levels(float* levels, float sampleRate);
