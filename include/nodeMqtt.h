#include <ArduinoJson.h>
#include <PubSubClient.h>

#include <sstream>
#include <iomanip>
#include <list>
#include <string>

#include "nodeLora.h"

const std::map<SensorType, std::string> sensorTypeMap =
    {
        {SensorType::temperature, "temperature"},
        {SensorType::humidity, "humidity"},
        {SensorType::voltage, "voltage"}
        // add a new pair here
};

class NodeMqtt
{
private:
    std::string _deviceName = "custom node";
    std::string _uniqueId = "626b3e3ff52"; // any randomly generated string
    std::string _stateTopic;

    // Variable used for MQTT Discovery
    const std::string _gatewayName = "lora_mqtt_gateway";

    const NodeLora &_nodeLora;

    std::string getJsonValueName(const std::string &devClass, uint8_t sensorIdx) const
    {
        return devClass + std::to_string(sensorIdx);
    }

public:
    NodeMqtt(const NodeLora &nodeLora);
    void publishDiscovery(PubSubClient &mqttClient) const;
    std::list<std::string> publishState(PubSubClient &mqttClient) const;
};

NodeMqtt::NodeMqtt(const NodeLora &nodeLora) : _nodeLora(nodeLora)
{
    _deviceName += "-" + std::to_string(nodeLora.getUniqueId());
    _uniqueId += "_" + std::to_string(nodeLora.getUniqueId());

    _stateTopic = _gatewayName + "/" + _deviceName + "/state";
}

void NodeMqtt::publishDiscovery(PubSubClient &mqttClient) const
{
    for (int i = 0; i < _nodeLora._sensors.size(); ++i)
    {
        const auto &sensor = _nodeLora._sensors[i];

        const std::string &devClass = sensorTypeMap.at(sensor.getType());

        String strPayload;
        JsonDocument payload;

        std::string uniqueSensorName = devClass + "-" + std::to_string(i);

        const std::string _discoveryTopic = "homeassistant/sensor/" + _uniqueId + "/" + uniqueSensorName + "/config";

        payload["name"] = _deviceName + " " + uniqueSensorName;
        payload["uniq_id"] = _uniqueId + "_" + uniqueSensorName + "_" + _gatewayName;
        payload["stat_t"] = _stateTopic;
        payload["dev_cla"] = devClass;
        payload["val_tpl"] = "{{value_json." + getJsonValueName(devClass, i) + "}}";
        payload["unit_of_meas"] = sensor.getDataUnit();
        payload["stat_cla"] = "measurement";

        JsonObject device = payload["dev"].to<JsonObject>();
        device["name"] = _deviceName;
        device["mf"] = "rusty-labs";
        JsonArray identifiers = device["ids"].to<JsonArray>();
        identifiers.add(_gatewayName + "_" + _uniqueId);

        serializeJson(payload, strPayload);

        mqttClient.publish(_discoveryTopic.c_str(), strPayload.c_str());

        Serial.println(strPayload);
    }
}

std::list<std::string> NodeMqtt::publishState(PubSubClient &mqttClient) const
{
    std::list<std::string> outList;
    JsonDocument payload;

    Serial.println("publishState");
    for (int i = 0; i < _nodeLora._sensors.size(); ++i)
    {
        const auto &sensor = _nodeLora._sensors[i];

        const std::string &devClass = sensorTypeMap.at(sensor.getType());
        std::string jsonValueName = getJsonValueName(devClass, i);

        switch (sensor.getDataType())
        {
        case SensorDataType::floatType:
        {
            float value = *reinterpret_cast<const float *>(sensor.getData().data());
            payload[jsonValueName] = value;

            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << value;
            std::string stringValue = ss.str();
            outList.emplace_back(jsonValueName + "=" + stringValue + " " + sensor.getDataUnit());
        }
        break;

        case SensorDataType::uint16_t:
        {
            uint16_t value = *reinterpret_cast<const uint16_t*>(sensor.getData().data());
            payload[jsonValueName] = value;            
            outList.emplace_back(jsonValueName + "=" + std::to_string(value) + " " + sensor.getDataUnit());
        }
        break;

        default:
            Serial.println("ERROR: Unknown data type");
        }
    }

    String strPayload;
    serializeJson(payload, strPayload);

    mqttClient.publish(_stateTopic.c_str(), strPayload.c_str());

    Serial.println(strPayload);

    return outList;
}