#include <Arduino.h>
#include <Preferences.h>

#include <lvgl.h>
#include <ui.h>
#include <WiFi.h>

#include "audio.h"
#include "display.h"
#include "http_server.h"
#include "input.h"
#include "lighting.h"
#include "wifi_manager.h"

Preferences preferences;
String endpointConfig;
String flameStatus = "On";

void setup()
{
    Serial.begin(115200);

    preferences.begin("config", false);
    endpointConfig = preferences.getString("endpoint", String());

    const bool wifiConnected = connectToWifi();
    initializeHttpServer(preferences, endpointConfig);

    initializeDisplay();
    const String wifiStatusText = wifiConnected ? "Connected: " + WiFi.localIP().toString() : "Failed to connect";
    showStartupStatus(wifiStatusText);
    playStartupTone();
    ui_init();
    initializeKeypad();

    Serial.println("Setup done");
}

void loop()
{
    lv_timer_handler();
    handleHttpServer();
    maintainWifiConnection();

    const String newStatus = readFlameStatusFromKeypad(flameStatus);
    if (newStatus != flameStatus)
    {
        flameStatus = newStatus;
        Serial.print("Flame status: ");
        Serial.println(flameStatus);
        updateLightingForStatus(flameStatus);
        updateAudioForStatus(flameStatus);
    }

    delay(100);
}
