#ifndef IDisplay_h
#define IDisplay_h

#include <array>
#include <WString.h>

class IDisplay
{
public:
    virtual void printLn(const char *msg) = 0;
    virtual void printLines(const std::array<String, 5> &lines) = 0;
};

#endif