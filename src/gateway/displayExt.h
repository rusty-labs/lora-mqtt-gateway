#include "HT_SSD1306Wire.h"
#include "IDisplay.h"

extern SSD1306Wire display;

class DisplayExt : public IDisplay
{
private:    

public:
    void printLn(const char *msg) override 
    {
        display.clear();
        display.drawString(0, 0, msg);
        display.display();
    }

    void printLines(const std::array<String, 5> &lines) override 
    {
        display.clear();

        for (int i = 0; i < lines.size(); i++)
        {
            display.drawString(1, 12 * i, lines[i]);
        }
        display.display();
    }

    void setup()
    {
        display.init();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);

        std::array<String, 5> displayLines = {
            "Bind button",
            " -Hold for 2 sec, release",
            "Clear and reset",
            " -Hold for 10 sec, release"
            };

        printLines(displayLines);
        delay(6000);
    }
};