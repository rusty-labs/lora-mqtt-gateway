#ifndef debug_h
#define debug_h

// #define DEBUG

#ifdef DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(...) Serial.printf(__VA_ARGS__)
#else
#define debug(x)
#define debugln(x)
#define debugf(...)
#endif

namespace Debug
{
#ifdef DEBUG
    static void setup()
    {
        Serial.begin(115200);
    }
#else
    static void setup() {}
#endif
}

#endif
