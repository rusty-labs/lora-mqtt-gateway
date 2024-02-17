#ifndef WifiManagerExt_h
#define WifiManagerExt_h

#include <functional>

#include <WiFi.h>
#include <Preferences.h>
#include "WiFiManager.h"

#include "debug.h"
#include "idisplay.h"

class WifiManagerExt
{
private:
    const char *_SSID = "LORA-MQTT-GATEWAY";
    const char *mqttServerId = "mqttServer";
    const char *mqttPortId = "mqttPort";
    const char *mqttUserId = "mqttUser";
    const char *mqttPasswordId = "mqttPassword";

    WiFiManager wm;
    WiFiManagerParameter paramMqttServer;
    WiFiManagerParameter paramMqttPort;
    WiFiManagerParameter paramMqttUser;
    WiFiManagerParameter paramMqttPassword;

    IDisplay &_display;

    Preferences _preferences;

    void saveConfigCallback()
    {
        _display.printLn("Restarting...");
        delay(3000);
        ESP.restart();
    }

    void displayIntro()
    {
        std::array<String, 5> displayLines = {
            "Connect to WiFi",
            String(_SSID),
            "Open ",
            "http://" + WiFi.softAPIP().toString(),
            ""};

        _display.printLines(displayLines);
    }

    void loadPreferences()
    {
        _preferences.begin("mqtt", false);

        String serverStr = _preferences.getString(mqttServerId, paramMqttServer.getValue());
        paramMqttServer.setValue(serverStr.c_str(), 128);

        String port = _preferences.getString(mqttPortId, paramMqttPort.getValue());
        paramMqttPort.setValue(port.c_str(), 5);

        String user = _preferences.getString(mqttUserId, paramMqttUser.getValue());
        paramMqttUser.setValue(user.c_str(), 128);

        String password = _preferences.getString(mqttPasswordId, paramMqttPassword.getValue());
        paramMqttPassword.setValue(password.c_str(), 128);
    }

    void savePreferences()
    {
        _preferences.putString(mqttServerId, paramMqttServer.getValue());
        _preferences.putString(mqttPortId, paramMqttPort.getValue());
        _preferences.putString(mqttUserId, paramMqttUser.getValue());
        _preferences.putString(mqttPasswordId, paramMqttPassword.getValue());

        _preferences.end();
    }

public:
    WifiManagerExt(IDisplay &display) : _display(display),
                                        paramMqttServer(mqttServerId, "MQTT Server", "homeassistant", 128),
                                        paramMqttPort(mqttPortId, "MQTT Port", "1883", 5),
                                        paramMqttUser(mqttUserId, "MQTT User", "homeassistant", 128),
                                        paramMqttPassword(mqttPasswordId, "MQTT Password", "", 128)
    {
    }

    void setupConfigurationPortal()
    {
        displayIntro();

        if (wm.getParametersCount() == 0)
        {
            debugln("Adding new parameters");
            wm.addParameter(&paramMqttServer);
            wm.addParameter(&paramMqttPort);
            wm.addParameter(&paramMqttUser);
            wm.addParameter(&paramMqttPassword);
        }

        wm.setBreakAfterConfig(true);

        debugln(WiFi.gatewayIP());
        debugln(WiFi.getHostname());
        debugln(WiFi.softAPIP());

        std::vector<const char *> wm_menu = {"wifi", "exit"};
        wm.setShowInfoUpdate(false);
        wm.setShowInfoErase(false);
        wm.setMenu(wm_menu);
    }

    void resetSettings()
    {
        wm.erase();

        _preferences.begin("mqtt", false);
        _preferences.clear();
        _preferences.end();
    }
    
    void setup()
    {
        loadPreferences();

        WiFi.mode(WIFI_STA);
        // wm.resetSettings();

        // WiFi.mode(WIFI_AP_STA); // explicitly set mode, esp defaults to STA+AP
        // wm.setConfigPortalTimeout(60);
        // wm.setConfigPortalBlocking(false);

        wm.setSaveConnect(false);
        wm.setConnectTimeout(5);
        wm.setSaveConfigCallback([&]()
                                 { saveConfigCallback(); });

        wm.setAPCallback([&](WiFiManager *)
                         { setupConfigurationPortal(); });

        wm.setSaveParamsCallback([&]()
                                 { savePreferences(); });

        _display.printLn("Connecting to WiFi...");
        bool res = wm.autoConnect(_SSID);

        if (!res)
        {
            _display.printLn("Failed to connect to WiFi!");
            debugln("Failed to connect to WiFi!");
            delay(3000);
            ESP.restart();
        }
        else
        {
            _display.printLn("Connected to WiFi");
            debugln("Connected to WiFi");
            delay(3000);
        }
    }

    void startConfigPortal()
    {
        if (!wm.startConfigPortal(_SSID))
        {
            _display.printLn("Failed to connect");
            delay(2000);
        }
    }

    WiFiClient &getClient()
    {
        return _wifiClient;
    }

    const char *getMqttServer()
    {
        return getValueById(mqttServerId, paramMqttServer.getValue());
    }

    const char *getMqttPort()
    {
        return getValueById(mqttPortId, paramMqttPort.getValue());
    }

    const char *getMqttUser()
    {
        return getValueById(mqttUserId, paramMqttUser.getValue());
    }

    const char *getMqttPassword()
    {
        return getValueById(mqttPasswordId, paramMqttPassword.getValue());
    }

    const char *getValueById(const char *id, const char *defaultValue)
    {
        for (int i = 0; i < wm.getParametersCount(); ++i)
        {
            WiFiManagerParameter *param = wm.getParameters()[i];
            if (strcmp(param->getID(), id) == 0)
            {
                return param->getValue();
            }
        }
        return defaultValue;
    }

    void loop()
    {
        /*
        if (!wm.process())
        {
            _displayMessage(("Connect to " + WiFi.SSID() + " WiFi").c_str(), 0);
            _displayMessage(("Open http://" + WiFi.softAPIP().toString()).c_str(), 1);

            debugln(WiFi.gatewayIP());
            debugln(WiFi.getHostname());
            debugln(WiFi.softAPIP());
        }
        */
    }

private:
    // TODO move to MQTT manager
    WiFiClient _wifiClient;
};

#endif