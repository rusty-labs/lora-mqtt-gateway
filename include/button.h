#ifndef button_h
#define button_h

#include "Arduino.h"
#include "debug.h"

class Button
{
public:
    Button(const int pin)
    {
        _pin = pin;
        _pressedState = LOW;
    }

    void setup()
    {
        pinMode(_pin, INPUT);
    }

    void process()
    {
        int state = digitalRead(_pin);

        if (state == _pressedState)
        {
            if (!_isPressed)
            {
                _holdTime = millis();
                _isPressed = true;
            }
        }
        else
        {
            if (_isPressed)
            {
                auto durationSincePressed = millis() - _holdTime;

                if (durationSincePressed > 50)
                {
                    debugln("Button is released!");
                    _onReleaseCallback(durationSincePressed / 1000);
                }
                _isPressed = false;
            }
        }
    }

    void setOnReleaseCallback(std::function<void(int64_t)> onReleaseCallback)
    {
        _onReleaseCallback = onReleaseCallback;
    }

    int64_t secondsSincePressed() const
    {
        if (_isPressed)
        {
            return (millis() - _holdTime) / 1000;
        }
        else
        {
            return 0;
        }
    }

private:
    uint32_t _holdTime;

    int _pin;
    int _pressedState;
    bool _isPressed = false;

    std::function<void(int64_t)> _onReleaseCallback;
};

#endif