#ifndef nodeLora_h
#define nodeLora_h

#undef min
#undef max

#include <vector>
#include <map>
#include "sensor.h"

uint8_t calculateXORChecksum(const uint8_t *data, uint16_t size)
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < size; i++)
    {
        checksum ^= data[i];
    }
    return checksum;
}

enum class NodeCommand : uint8_t
{
    handshake,
    state
    // add a new command here
};

class NodeLora
{
public:
    template <class T>
    void addSensor(SensorType sensType, SensorDataType sensDataType, const std::string &dataUnit, const T &data)
    {
        _sensors.emplace_back(sensType, sensDataType, dataUnit, data);
    }

    std::array<uint8_t, 4> _fixedPropVec{'r', 'y', 0, 0};
    std::vector<Sensor> _sensors;

    NodeLora() {}

    NodeLora(uint8_t uniqueId, NodeCommand command)
    {
        _fixedPropVec[2] = uniqueId; // up to 255 nodes can be used, each node should have a unique id, each node can contain up to 255 sensors
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
        Serial.printf("\r\n");
        for (int i = 0; i < size; ++i)
        {
            Serial.printf("%d ", payload[i]);
        }
        Serial.printf("\r\n");

        uint16_t minPayloadSize = sizeof(_fixedPropVec) + Sensor::getMinSize();

        Serial.printf("minPayloadSize %d size %d\r\n", minPayloadSize, size);

        Serial.printf("p0 %d\r\n", _fixedPropVec[0]);
        Serial.printf("p1 %d\r\n", _fixedPropVec[1]);

        if (size < minPayloadSize || payload[0] != _fixedPropVec[0] || payload[1] != _fixedPropVec[1])
        {
            Serial.printf("here1\r\n");
            return false;
        }

        // checksum check
        uint8_t actualCheckSum = calculateXORChecksum(payload, size - sizeof(actualCheckSum));
        uint8_t payloadCheckSum = *(payload + size - sizeof(payloadCheckSum));

        Serial.printf("sum0 %d\r\n", actualCheckSum);
        Serial.printf("sum1 %d\r\n", payloadCheckSum);

        if (payloadCheckSum != actualCheckSum)
        {
            Serial.printf("here2\r\n");
            return false;
        }

        _sensors.clear();

        uint16_t pos = 0;

        std::memcpy(_fixedPropVec.data(), payload + pos, _fixedPropVec.size());
        pos += _fixedPropVec.size();

        const uint8_t sizeNoCheckSum = size - sizeof(payloadCheckSum);

        while (pos < sizeNoCheckSum - 1)
        {
            Sensor newSensor;
            pos += newSensor.copyFrom(payload + pos);
            _sensors.emplace_back(newSensor);
        }

        return true;
    }

    std::vector<uint8_t> encode()
    {
        uint8_t checksum;

        uint16_t messageSize = sizeof(_fixedPropVec);

        for (const auto &attr : _sensors)
        {
            messageSize += attr.getSize();
        }

        messageSize += sizeof(checksum);

        std::vector<uint8_t> outMessage(messageSize);
        uint16_t pos = 0;

        memcpy(outMessage.data() + pos, _fixedPropVec.data(), sizeof(_fixedPropVec));
        pos += sizeof(_fixedPropVec);

        for (const auto &attr : _sensors)
        {
            pos += attr.copyTo(outMessage.data() + pos);
        }

        checksum = calculateXORChecksum(outMessage.data(), outMessage.size() - sizeof(checksum));
        memcpy(outMessage.data() + pos, &checksum, sizeof(checksum));

        Serial.printf("\r\n");
        for (int i = 0; i < outMessage.size(); ++i)
        {
            Serial.printf("%d ", outMessage[i]);
        }
        Serial.printf("\r\n");

        return outMessage;
    }
};

#endif