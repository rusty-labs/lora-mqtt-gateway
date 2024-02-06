#ifndef nodeLora_h
#define nodeLora_h

#undef min
#undef max

#include <vector>
#include <map>

#include "debug.h"
#include "sensor.h"

enum class NodeCommand : uint8_t
{
    discovery,
    state
    // add a new command here
};

class NodeLora
{
public:
    template <class T>
    void addSensor(SensorType type, SensorDataType dataType, const std::string &dataUnit, const T &data)
    {
        _sensors.emplace_back(type, dataType, dataUnit, data);
    }

    std::array<uint8_t, 4> _fixedPropVec{'r', 'y', 0, 0};

    // Each node can contain up to 255 sensors
    std::vector<Sensor> _sensors;

    NodeLora() {}

    // uniqueId, each node should have an unique id
    NodeLora(uint8_t uniqueId, NodeCommand command)
    {
        _fixedPropVec[2] = uniqueId;
        _fixedPropVec[3] = static_cast<uint8_t>(command);
    }

    NodeCommand getCommand() const
    {
        return static_cast<NodeCommand>(_fixedPropVec[3]);
    }

    uint8_t getUniqueId() const
    {
        return _fixedPropVec[2];
    }

    bool decode(uint8_t *payload, const uint16_t size)
    {
        debugf("\r\n");
        for (int i = 0; i < size; ++i)
        {
            debugf("%d ", payload[i]);
        }
        debugf("\r\n");

        uint16_t minPayloadSize = sizeof(_fixedPropVec) + Sensor::getMinSize();

        debugf("minPayloadSize %d size %d\r\n", minPayloadSize, size);

        debugf("p0 %d\r\n", _fixedPropVec[0]);
        debugf("p1 %d\r\n", _fixedPropVec[1]);

        if (size < minPayloadSize || payload[0] != _fixedPropVec[0] || payload[1] != _fixedPropVec[1])
        {
            debugf("here1\r\n");
            return false;
        }

        _sensors.clear();

        uint16_t pos = 0;

        std::memcpy(_fixedPropVec.data(), payload + pos, _fixedPropVec.size());
        pos += _fixedPropVec.size();

        while (pos < size - 1)
        {
            Sensor newSensor;
            pos += newSensor.copyFrom(payload + pos);
            _sensors.emplace_back(newSensor);
        }

        return true;
    }

    std::vector<uint8_t> encode()
    {
        uint16_t messageSize = sizeof(_fixedPropVec);

        for (const auto &attr : _sensors)
        {
            messageSize += attr.getSize();
        }

        std::vector<uint8_t> outMessage(messageSize);
        uint16_t pos = 0;

        memcpy(outMessage.data() + pos, _fixedPropVec.data(), sizeof(_fixedPropVec));
        pos += sizeof(_fixedPropVec);

        for (const auto &attr : _sensors)
        {
            pos += attr.copyTo(outMessage.data() + pos);
        }

        debugf("\r\n");
        for (int i = 0; i < outMessage.size(); ++i)
        {
            debugf("%d ", outMessage[i]);
        }
        debugf("\r\n");

        return outMessage;
    }
};

#endif