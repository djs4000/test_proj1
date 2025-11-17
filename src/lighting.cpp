#include "lighting.h"

void updateLightingForStatus(const String &status)
{
    // TODO: Map status values to WS2812 animations.
    // Keeping a log here allows future LED drivers to hook in without changing callers.
    Serial.print("Lighting update for status: ");
    Serial.println(status);
}
