#include "input.h"

#include <Keypad.h>

namespace
{
constexpr byte kKeypadRows = 4;
constexpr byte kKeypadCols = 4;

char hexaKeys[kKeypadRows][kKeypadCols] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

byte rowPins[kKeypadCols] = {12, 14, 27, 26};
byte colPins[kKeypadRows] = {25, 33, 32, 18};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, kKeypadRows, kKeypadCols);
}

void initializeKeypad()
{
    // Nothing to initialize beyond constructing the keypad object for now.
}

String readFlameStatusFromKeypad(const String &currentStatus)
{
    const char customKey = customKeypad.getKey();
    if (!customKey)
    {
        return currentStatus;
    }

    if (customKey == 'A')
    {
        Serial.println("Key Pressed: A");
        return "Armed";
    }
    if (customKey == 'B')
    {
        Serial.println("Key Pressed: B");
        return "Defused";
    }
    if (customKey == 'C')
    {
        Serial.println("Key Pressed: C");
        return "Exploded";
    }

    Serial.print("Key Pressed: ");
    Serial.println(customKey);
    return "On";
}
