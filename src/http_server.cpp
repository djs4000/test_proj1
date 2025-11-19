#include "http_server.h"

#include <WebServer.h>

#include "utils.h"

namespace
{
WebServer server(80);
Preferences *preferencesPtr = nullptr;
String *endpointConfigPtr = nullptr;
constexpr uint16_t kRestartGracePeriodMs = 500;
bool restartPending = false;
unsigned long restartRequestedMs = 0;

void sendConfigurationPage(const String &message = String())
{
    String html = F("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Device Configuration</title><style>body{font-family:Arial,sans-serif;margin:40px;}label{display:block;margin-bottom:8px;font-weight:bold;}input[type=text]{width:100%;max-width:480px;padding:8px;margin-bottom:12px;}button{padding:10px 16px;margin-right:10px;font-size:14px;cursor:pointer;}form{margin-bottom:20px;}p.status{color:green;}</style></head><body><h1>Device Configuration</h1>");
    if (message.length() > 0)
    {
        html += F("<p class='status'>");
        html += htmlEscape(message);
        html += F("</p>");
    }
    html += F("<form method='POST' action='/save'><label for='endpoint'>Endpoint URL</label><input type='text' id='endpoint' name='endpoint' value='");
    html += htmlEscape(*endpointConfigPtr);
    html += F("' placeholder='https://example.com/api'><button type='submit'>Save</button></form><form method='POST' action='/restart'><button type='submit'>Restart Device</button></form></body></html>");
    server.send(200, "text/html", html);
}

void handleRoot()
{
    sendConfigurationPage();
}

void handleSave()
{
    if (server.hasArg("endpoint"))
    {
        *endpointConfigPtr = server.arg("endpoint");
        preferencesPtr->putString("endpoint", *endpointConfigPtr);
        sendConfigurationPage(F("Configuration saved."));
    }
    else
    {
        server.send(400, "text/plain", "Missing endpoint parameter");
    }
}

void handleRestart()
{
    server.send(200, "text/html", F("<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'><title>Restarting</title></head><body><h1>Device Restarting</h1><p>The device will restart to apply the new settings.</p></body></html>"));
    restartPending = true;
    restartRequestedMs = millis();
}
}

void initializeHttpServer(Preferences &preferences, String &endpointConfig)
{
    preferencesPtr = &preferences;
    endpointConfigPtr = &endpointConfig;

    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/restart", HTTP_POST, handleRestart);
    server.begin();
}

void handleHttpServer()
{
    server.handleClient();

    if (restartPending && millis() - restartRequestedMs >= kRestartGracePeriodMs)
    {
        restartPending = false;
        ESP.restart();
    }
}
