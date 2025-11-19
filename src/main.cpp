#include <Arduino.h>
#include <Preferences.h>

#include <lvgl.h>
#include <ui.h>

#include "audio.h"
#include "display.h"
#include "http_server.h"
#include "input.h"
#include "lighting.h"
#include "wifi_manager.h"

namespace
{
constexpr uint16_t kKeypadPollIntervalMs = 100;
unsigned long s_lastKeypadPollMs = 0;
}

Preferences preferences;
String endpointConfig;
String flameStatus = "On";

void setup()
{
    Serial.begin(115200);

    preferences.begin("config", false);
    endpointConfig = preferences.getString("endpoint", String());

    connectToWifi();
    initializeHttpServer(preferences, endpointConfig);

    initializeDisplay();
    ui_init();
    initializeKeypad();

    Serial.println("Setup done");
}

void loop()
{
    lv_timer_handler();
    handleHttpServer();
    maintainWifiConnection();

    const unsigned long now = millis();
    if (now - s_lastKeypadPollMs >= kKeypadPollIntervalMs)
    {
        s_lastKeypadPollMs = now;

        const String newStatus = readFlameStatusFromKeypad(flameStatus);
        if (newStatus != flameStatus)
        {
            flameStatus = newStatus;
            Serial.print("Flame status: ");
            Serial.println(flameStatus);
            updateLightingForStatus(flameStatus);
            updateAudioForStatus(flameStatus);
        }
    }
}
