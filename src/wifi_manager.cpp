#include "wifi_manager.h"

#include <WiFi.h>

#include "wifi_config.h"

namespace
{
constexpr uint8_t kMaxConnectionAttempts = 20;
constexpr uint16_t kRetryDelayMs = 500;
constexpr uint16_t kReconnectCooldownMs = 1500;

uint8_t s_attempts = 0;
unsigned long s_lastAttemptMs = 0;
unsigned long s_nextReconnectMs = 0;
bool s_waitingForConnection = false;
bool s_reportedConnection = false;

void startConnectionAttempt()
{
    Serial.printf("Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    s_attempts = 0;
    s_lastAttemptMs = millis();
    s_waitingForConnection = true;
}
}

void connectToWifi()
{
    s_nextReconnectMs = 0;
    startConnectionAttempt();
}

void maintainWifiConnection()
{
    const wl_status_t status = WiFi.status();
    const unsigned long now = millis();

    if (status == WL_CONNECTED)
    {
        if (!s_reportedConnection)
        {
            Serial.println();
            Serial.print("WiFi connected. IP address: ");
            Serial.println(WiFi.localIP());
            s_reportedConnection = true;
        }
        s_waitingForConnection = false;
        return;
    }

    s_reportedConnection = false;

    if (!s_waitingForConnection)
    {
        if (s_nextReconnectMs == 0 || now >= s_nextReconnectMs)
        {
            Serial.println();
            Serial.println("WiFi disconnected. Attempting reconnection.");
            startConnectionAttempt();
        }
        return;
    }

    if (s_attempts >= kMaxConnectionAttempts)
    {
        Serial.println();
        Serial.println("Failed to connect to WiFi. Check credentials and signal strength.");
        s_waitingForConnection = false;
        s_nextReconnectMs = now + kReconnectCooldownMs;
        return;
    }

    if (now - s_lastAttemptMs >= kRetryDelayMs)
    {
        Serial.print('.');
        WiFi.reconnect();
        s_lastAttemptMs = now;
        s_attempts++;
    }
}
