#pragma once

#include <Arduino.h>

void initializeKeypad();
String readFlameStatusFromKeypad(const String &currentStatus);
