#include "wifi_manager.h"

#include <WiFi.h>

#include "wifi_config.h"

namespace
{
constexpr uint8_t kMaxConnectionAttempts = 20;
constexpr uint16_t kRetryDelayMs = 500;
}

void connectToWifi()
{
    Serial.printf("Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < kMaxConnectionAttempts)
    {
        delay(kRetryDelayMs);
        Serial.print('.');
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("WiFi connected. IP address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println();
        Serial.println("Failed to connect to WiFi. Check credentials and signal strength.");
    }
}

void maintainWifiConnection()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        connectToWifi();
        delay(1500);
    }
}
