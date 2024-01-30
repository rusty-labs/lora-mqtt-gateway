#include <vector>
#include <string>
#include <cstring>

enum class SensorType : uint8_t
{
    temperature,
    humidity,
    voltage
    // add a new type here
};

enum class SensorDataType : uint8_t
{
    floatType
    // add a new data type here
};

struct Sensor
{
    // fixed size properties
    std::array<uint8_t, 2> _fixedPropVec;

    // variable size propeties
    std::array<std::vector<uint8_t>, 2> _variablePropVec;

    Sensor() {}

    template <class T>
    Sensor(SensorType type, SensorDataType dataType, const std::string &dataUnit, const T &data)
    {
        _fixedPropVec[0] = static_cast<uint8_t>(type);
        _fixedPropVec[1] = static_cast<uint8_t>(dataType);

        setVariableData(0, dataUnit);
        setVariableData(1, data);
    }

    SensorType getType() const
    {
        return (SensorType)_fixedPropVec[0];
    }

    SensorDataType getDataType() const
    {
        return (SensorDataType)_fixedPropVec[1];
    }

    std::string getDataUnit() const
    {
        return std::string(_variablePropVec[0].data(), _variablePropVec[0].data() + _variablePropVec[0].size());
    }

    std::vector<uint8_t> getData() const
    {
        return _variablePropVec[1];
    }

    template <class T>
    void setVariableData(uint8_t idx, const T &data)
    {
        _variablePropVec[idx] = std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(&data), reinterpret_cast<const uint8_t*>(&data) + sizeof(data));        
    }

    static uint8_t getMinSize()
    {
        return sizeof(_fixedPropVec) + sizeof(uint8_t);
    }

    uint8_t getSize() const
    {
        uint8_t size = sizeof(_fixedPropVec);

        for (const auto &prop : _variablePropVec)
        {
            size += sizeof(uint8_t) + prop.size();
        }

        return size;
    }

    uint8_t copyTo(uint8_t *outputBuffer) const
    {
        uint8_t pos = 0;

        std::memcpy(outputBuffer + pos, _fixedPropVec.data(), _fixedPropVec.size() * sizeof(uint8_t));
        pos += _fixedPropVec.size() * sizeof(uint8_t);

        for (const auto &prop : _variablePropVec)
        {
            const uint8_t sz = prop.size();
            std::memcpy(outputBuffer + pos, &sz, sizeof(uint8_t));
            pos += sizeof(uint8_t);

            std::memcpy(outputBuffer + pos, prop.data(), prop.size());
            pos += sz;
        }

        return pos;
    }

    uint8_t copyFrom(const uint8_t *inputBuffer)
    {
        uint8_t pos = 0;

        std::memcpy(_fixedPropVec.data(), inputBuffer + pos, _fixedPropVec.size() * sizeof(uint8_t));
        pos += _fixedPropVec.size() * sizeof(uint8_t);

        for (auto &prop : _variablePropVec)
        {
            const uint8_t sz = *reinterpret_cast<const uint8_t *>(inputBuffer + pos);
            pos += sizeof(sz);

            prop = std::vector<uint8_t>((inputBuffer + pos), (inputBuffer + pos) + sz);
            pos += sz;
        }

        return pos;
    }
};

template <>
void Sensor::setVariableData<std::string>(uint8_t idx, const std::string &data)
{
    _variablePropVec[idx] = std::vector<uint8_t>(data.data(), data.data() + data.size());
}