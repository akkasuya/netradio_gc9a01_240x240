// I2S PCM tap for the visualizer. Must not include Audio.h: that header
// declares audio_process_i2s() as weak, which would make this definition weak too.
#include "spectrum.h"
#include <math.h>
#include <string.h>

static int32_t spec_window[SPEC_N];
static volatile uint16_t spec_count = 0;
static volatile bool spec_ready = false;

void audio_process_i2s(int32_t* outBuff, int16_t validSamples, bool* continueI2S) {
    (void)continueI2S;
    if (spec_ready) return;
    for (int i = 0; i < validSamples && spec_count < SPEC_N; i++) {
        int32_t mono = (outBuff[i * 2] >> 1) + (outBuff[i * 2 + 1] >> 1);
        spec_window[spec_count++] = mono;
        if (spec_count >= SPEC_N) {
            spec_ready = true;
            break;
        }
    }
}

bool spectrum_levels(float* levels, float sampleRate) {
    static int32_t win[SPEC_N];
    static float refPeak = 1.0f;

    if (!spec_ready) return false;
    memcpy(win, spec_window, sizeof(win));
    spec_count = 0;
    spec_ready = false;

    int32_t peak = 0;
    for (int i = 0; i < SPEC_N; i++) {
        int32_t a = win[i] >= 0 ? win[i] : -win[i];
        if (a > peak) peak = a;
    }
    if (peak < 500) {
        for (int b = 0; b < SPEC_BANDS; b++) levels[b] = 0.0f;
        return true;
    }

    // Slow-decaying reference so bars follow loudness changes without full-scale flattening
    refPeak = fmaxf((float)peak, refPeak * 0.995f);
    float norm = 1.0f / refPeak;
    if (sampleRate <= 0) sampleRate = 44100.0f;

    for (int b = 0; b < SPEC_BANDS; b++) {
        // log-spaced bands, 100 Hz .. 8 kHz
        float f = 100.0f * powf(80.0f, (float)b / (SPEC_BANDS - 1));
        float coeff = 2.0f * cosf(2.0f * 3.14159265f * f / sampleRate);
        float s1 = 0, s2 = 0;
        for (int n = 0; n < SPEC_N; n++) {
            float s0 = win[n] * norm + coeff * s1 - s2;
            s2 = s1;
            s1 = s0;
        }
        float mag = sqrtf(fmaxf(s1 * s1 + s2 * s2 - coeff * s1 * s2, 0.0f)) / (SPEC_N / 2);
        float db = 20.0f * log10f(mag + 1e-6f);
        float level = (db + 50.0f) / 50.0f;
        levels[b] = level < 0.0f ? 0.0f : (level > 1.0f ? 1.0f : level);
    }
    return true;
}
