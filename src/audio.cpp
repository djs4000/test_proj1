#include "audio.h"

namespace
{
// The CYD routes its built-in amplified DAC output to GPIO 26.
constexpr uint8_t kBuzzerPin = 26;
constexpr uint8_t kBuzzerChannel = 0;
constexpr uint16_t kStartupToneDurationMs = 200;
constexpr double kStartupToneFrequencyHz = 2000.0;
}

void playStartupTone()
{
    ledcSetup(kBuzzerChannel, kStartupToneFrequencyHz, 10);
    ledcAttachPin(kBuzzerPin, kBuzzerChannel);
    ledcWriteTone(kBuzzerChannel, kStartupToneFrequencyHz);
    delay(kStartupToneDurationMs);
    ledcWriteTone(kBuzzerChannel, 0);
}

void updateAudioForStatus(const String &status)
{
    // TODO: Route status changes to audio cues when a speaker driver is added.
    Serial.print("Audio update for status: ");
    Serial.println(status);
}
