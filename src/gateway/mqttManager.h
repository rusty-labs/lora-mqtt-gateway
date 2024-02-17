#include <functional>
#include <set>

#include <PubSubClient.h>

#include "idisplay.h"
#include "nodeMqtt.h"
#include "wifiManagerExt.h"

class MqttManager
{
private:
    PubSubClient mqttClient;

    std::set<uint8_t> sensorSet;

    IDisplay &_display;

    WifiManagerExt &_wifiManager;

private:
    void mqttReceiverCallback(char *topic, byte *inFrame, unsigned int length)
    {
        debug("Message arrived on topic: ");
        debug(topic);
        debug(". Message: ");

        std::string messageTemp;

        for (int i = 0; i < length; i++)
        {
            debug((char)inFrame[i]);
            messageTemp += (char)inFrame[i];
        }
        debugln();

        if (std::string(topic) == std::string("homeassistant/status"))
        {
            if (messageTemp == "online")
            {
                // send discovery if home assistant was rebooted
                sensorSet.clear();
            }
        }
    }

public:
    MqttManager(WifiManagerExt &wifiManager, IDisplay &display) : _wifiManager(wifiManager), mqttClient(wifiManager.getClient()), _display(display)
    {
    }

    bool connect()
    {
        if (mqttClient.connected())
        {
            return true;
        }

        int attempts = 0;

        String progressLine = "Connecting to MQTT";

        while (!mqttClient.connected() && attempts < 10)
        {
            _display.printLn(progressLine.c_str());
            debug("Attempting MQTT connection...");
            // Attempt to connect
            if (mqttClient.connect("LoraMqttGateway", _wifiManager.getMqttUser(), _wifiManager.getMqttPassword()))
            {
                _display.printLn("Connected to MQTT ");
                debugln("connected");

                mqttClient.subscribe("homeassistant/status");
                return true;
            }
            else
            {
                debug("failed, rc=");
                debugln(mqttClient.state());
                delay(1000);
            }

            ++attempts;
            progressLine += ".";
        }

        std::array<String, 5> displayLines = {
            "MQTT connection failed!",
            "Check MQTT settings"};

        _display.printLines(displayLines);
        delay(3000);

        return false;
    }

    void setup()
    {
        mqttClient.setServer(_wifiManager.getMqttServer(), atoi(_wifiManager.getMqttPort()));
        mqttClient.setCallback([&](char *topic, byte *inFrame, unsigned int length)
                               { mqttReceiverCallback(topic, inFrame, length); });
        mqttClient.setBufferSize(2048);
    }

    std::list<std::string> publish(const NodeLora &nodeLora)
    {
        NodeMqtt nodeMqtt(nodeLora);

        if (nodeLora.getCommand() == NodeCommand::discovery)
        {
            // send discovery if sensor rebooted with new settings
            nodeMqtt.publishDiscovery(mqttClient);
        }
        else
        {
            auto it = sensorSet.find(nodeLora.getUniqueId());
            if (it == sensorSet.end())
            {
                sensorSet.emplace(nodeLora.getUniqueId());
                // send discovery if gateway rebooted
                nodeMqtt.publishDiscovery(mqttClient);
            }
        }

        auto newSensorsList = nodeMqtt.publishState(mqttClient);
        return newSensorsList;
    }

    void loop()
    {
        mqttClient.loop();
    }
};