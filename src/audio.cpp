#include "audio.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <driver/i2s.h>

namespace
{
    constexpr i2s_port_t kPort = I2S_NUM_0;
    constexpr int kSampleRate = 22050;
    constexpr float kStartupToneFrequencyHz = 880.0F;
    constexpr float kStartupToneDurationSeconds = 0.15F;
    constexpr float kPi = 3.14159265358979323846F;

    uint16_t biasForDac(float normalizedSample)
    {
        const float clamped = std::max(-1.0F, std::min(1.0F, normalizedSample));
        const uint8_t dacLevel = static_cast<uint8_t>((clamped * 127.0F) + 128.0F);
        return static_cast<uint16_t>(dacLevel) << 8;
    }
}

void updateAudioForStatus(const String &status)
{
    // TODO: Route status changes to audio cues when a speaker driver is added.
    Serial.print("Audio update for status: ");
    Serial.println(status);
}

void playStartupTone()
{
    const i2s_config_t config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = kSampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
    };

    if (i2s_driver_install(kPort, &config, 0, nullptr) != ESP_OK)
    {
        Serial.println("Failed to start I2S driver for DAC");
        return;
    }

    // Built-in DAC does not need pin mapping; ensure both channels are enabled to keep DAC2 (GPIO26) active.
    i2s_set_pin(kPort, nullptr);
    i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
    i2s_set_clk(kPort, kSampleRate, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
    i2s_zero_dma_buffer(kPort);

    const size_t samples = static_cast<size_t>(kSampleRate * kStartupToneDurationSeconds);
    const float radiansPerSample = (2.0F * kPi * kStartupToneFrequencyHz) / static_cast<float>(kSampleRate);

    // Stereo frames for the DAC expect right + left samples, each biased to 8-bit and stored in the upper byte of a 16-bit word.
    std::vector<uint16_t> frames;
    frames.reserve(samples * 2);

    for (size_t i = 0; i < samples; ++i)
    {
        const float sample = std::sin(radiansPerSample * static_cast<float>(i));
        const uint16_t dacSample = biasForDac(sample);
        frames.push_back(dacSample); // Right channel (GPIO26 / DAC2)
        frames.push_back(dacSample); // Left channel  (GPIO25 / DAC1)
    }

    size_t bytesWritten = 0;
    i2s_write(kPort, frames.data(), frames.size() * sizeof(uint16_t), &bytesWritten, portMAX_DELAY);
    i2s_stop(kPort);
    i2s_driver_uninstall(kPort);
}
