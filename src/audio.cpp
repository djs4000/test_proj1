#include "audio.h"

#include <algorithm>
#include <cmath>

#include "driver/i2s.h"

namespace
{
constexpr uint16_t kStartupToneDurationMs = 200;
constexpr double kStartupToneFrequencyHz = 2000.0;
constexpr uint32_t kSampleRateHz = 16000;
constexpr float kToneAmplitude = 120.0f; // 8-bit DAC amplitude headroom, shifted into MSB for I2S
constexpr double kTwoPi = 6.283185307179586;

bool configureBuiltInDac()
{
    const i2s_config_t config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = static_cast<int>(kSampleRateHz),
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
        .mclk_multiple = I2S_MCLK_MULTIPLE_DEFAULT,
        .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT,
    };

    if (i2s_driver_install(I2S_NUM_0, &config, 0, nullptr) != ESP_OK)
    {
        Serial.println("Failed to install I2S driver for DAC output");
        return false;
    }

    // Route the startup tone through the CYD's amplified DAC on GPIO 26.
    i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN);
    i2s_set_sample_rates(I2S_NUM_0, kSampleRateHz);
    i2s_zero_dma_buffer(I2S_NUM_0);

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

        size_t bytesWritten = 0;
        i2s_write(I2S_NUM_0, buffer, samplesThisChunk * sizeof(uint16_t), &bytesWritten, portMAX_DELAY);
    }

    i2s_driver_uninstall(I2S_NUM_0);
}

void updateAudioForStatus(const String &status)
{
    // TODO: Route status changes to audio cues when a speaker driver is added.
    Serial.print("Audio update for status: ");
    Serial.println(status);
}
