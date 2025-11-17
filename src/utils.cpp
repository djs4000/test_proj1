#include "utils.h"

String htmlEscape(const String &input)
{
    String escaped;
    escaped.reserve(input.length());
    for (uint16_t i = 0; i < input.length(); ++i)
    {
        const char c = input.charAt(i);
        switch (c)
        {
        case '&':
            escaped += F("&amp;");
            break;
        case '<':
            escaped += F("&lt;");
            break;
        case '>':
            escaped += F("&gt;");
            break;
        case 39:
            escaped += F("&#39;");
            break;
        case '"':
            escaped += F("&quot;");
            break;
        default:
            escaped += c;
            break;
        }
    }
    return escaped;
}
