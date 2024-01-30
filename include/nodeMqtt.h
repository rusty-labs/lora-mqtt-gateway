#include <ArduinoJson.h>
#include <PubSubClient.h>

#include <list>
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
    std::string _deviceName = "custom sensor";
    std::string _uniqueId = "626b3e3ff52";
    std::string _stateTopic;

    // Variable used for MQTT Discovery
    const std::string _gatewayName = "lora_mqtt_gateway";

    const NodeLora &_nodeLora;

public:
    NodeMqtt(const NodeLora &nodeLora);
    void publishDiscovery(PubSubClient &mqttClient) const;
    std::list<std::string> publishState(PubSubClient &mqttClient) const;
};

NodeMqtt::NodeMqtt(const NodeLora &nodeLora) : _nodeLora(nodeLora)
{
    _deviceName += " " + std::to_string(nodeLora.getUniqueId());
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

        std::string uniqueSensorName = devClass + std::to_string(i);

        const std::string _discoveryTopic = "homeassistant/sensor/" + _uniqueId + "/" + uniqueSensorName + "/config";

        payload["name"] = _deviceName + " " + uniqueSensorName;
        payload["uniq_id"] = _uniqueId + "_" + uniqueSensorName + "_" + _gatewayName;
        payload["stat_t"] = _stateTopic;
        payload["dev_cla"] = devClass;
        payload["val_tpl"] = "{{value_json." + uniqueSensorName + "}}";
        payload["unit_of_meas"] = sensor.getDataUnit();
        payload["stat_cla"] = "measurement";

        JsonObject device = payload["device"].to<JsonObject>();
        device["name"] = _deviceName;
        JsonArray identifiers = device["identifiers"].to<JsonArray>();
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
        std::string uniqueSensorName = devClass + std::to_string(i);

        switch (sensor.getDataType())
        {
        case SensorDataType::floatType:
        {
            float value = *reinterpret_cast<const float *>(sensor.getData().data());
            payload[uniqueSensorName] = value;
            outList.emplace_back(uniqueSensorName + "=" + std::to_string(value));
        }
        break;

        default:
            Serial.println("ERROR: Unknown data type");
        }
    }

    Serial.println("payload");
    String strPayload;
    serializeJson(payload, strPayload);

    mqttClient.publish(_stateTopic.c_str(), strPayload.c_str());

    Serial.println(strPayload);

    return outList;
}