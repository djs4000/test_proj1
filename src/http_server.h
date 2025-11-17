#pragma once

#include <Arduino.h>
#include <Preferences.h>

void initializeHttpServer(Preferences &preferences, String &endpointConfig);
void handleHttpServer();
