#include "audio_module.h"

#include <Audio.h>
#include <I2S.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr uint16_t kStartupToneDurationMs = 200;
constexpr double kStartupToneFrequencyHz = 2000.0;
constexpr uint32_t kSampleRateHz = 16000;
constexpr float kToneAmplitude = 120.0f; // 8-bit DAC amplitude headroom, shifted into MSB for I2S
constexpr double kTwoPi = 6.283185307179586;

bool configureBuiltInDac()
{
    // Use the Arduino I2S wrapper so the sketch relies on the standard PlatformIO
    // library instead of the raw ESP-IDF driver calls.
    if (!I2S.begin(I2S_MODE_DAC_BUILT_IN,
                   kSampleRateHz,
                   I2S_BITS_PER_SAMPLE_16BIT,
                   I2S_CHANNEL_FMT_ONLY_LEFT))
    {
        Serial.println("Failed to start I2S for DAC output");
        return false;
    }

    // Route the startup tone through the CYD's amplified DAC on GPIO 26 using the
    // Arduino I2S helper and its DAC mode configuration.
    I2S.setPins(I2S_PIN_NO_CHANGE, I2S_PIN_NO_CHANGE, I2S_PIN_NO_CHANGE, I2S_PIN_NO_CHANGE);
    I2S.setDACMode(I2S_DAC_CHANNEL_LEFT_EN);
    I2S.flush();

    return true;
}
}

void playStartupTone()
{
    if (!configureBuiltInDac())
    {
        return;
    }

    const uint32_t totalSamples = (kSampleRateHz * kStartupToneDurationMs) / 1000;
    constexpr size_t kChunkSamples = 256;
    uint16_t buffer[kChunkSamples];

    for (uint32_t generated = 0; generated < totalSamples; generated += kChunkSamples)
    {
        const size_t samplesThisChunk = std::min<size_t>(kChunkSamples, totalSamples - generated);
        for (size_t i = 0; i < samplesThisChunk; ++i)
        {
            const double phase = kTwoPi * kStartupToneFrequencyHz * static_cast<double>(generated + i) / kSampleRateHz;
            const uint16_t sample = static_cast<uint16_t>((std::sin(phase) * kToneAmplitude + 128.0f)) << 8;
            buffer[i] = sample;
        }

        I2S.write(reinterpret_cast<uint8_t *>(buffer), samplesThisChunk * sizeof(uint16_t));
    }

    I2S.end();
}

void updateAudioForStatus(const String &status)
{
    // TODO: Route status changes to audio cues when a speaker driver is added.
    Serial.print("Audio update for status: ");
    Serial.println(status);
}
