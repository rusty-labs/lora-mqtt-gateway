#include <functional>

#include "loraSettings.h"
#include "medianFilter.h"
#include "nodeLora.h"

#include "LoRaWan_APP.h"
#include "Wire.h"

#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
/*
 * set LoraWan_RGB to 1,the RGB active in loraWan
 * RGB red means sending;
 * RGB green means received done;
 */
#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

void OnTxDone();
void OnTxTimeout();

enum class STATES : uint8_t
{
  LOW_POWER,
  READ_SENSORS,
  TX
};

STATES state;

uint16_t voltage = 0;
float objectTemperature = 0;
float ambientTemperature = 0;

bool firstRun = true;

void initMlx()
{
  if (!mlx.begin())
  {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1)
    {
      delay(1000);
    }
  };

  Serial.print("Emissivity = ");
  Serial.println(mlx.readEmissivity());
  Serial.println("================================================");
}

void setup()
{
  Serial.begin(115200);
  
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  state = STATES::READ_SENSORS;

  initMlx();
}

template<class T>
T getMedianValue(std::function<T()> getValueFunction)
{
      MedianFilter<T, 5> medianFilter;
      for(uint32_t i = 0; i<medianFilter.size(); ++i)
      {
        medianFilter.set(i, getValueFunction());
      }
      return medianFilter.getMedian();
}

void loop()
{
  switch (state)
  {
  case STATES::TX:
  {
    turnOnRGB(COLOR_SEND, 0);

    NodeLora nodeLora(0, firstRun ? NodeCommand::discovery : NodeCommand::state);    
    firstRun = false;

    nodeLora.addSensor(SensorType::temperature, SensorDataType::floatType, "°C", ambientTemperature);
    nodeLora.addSensor(SensorType::temperature, SensorDataType::floatType, "°C", objectTemperature);
    nodeLora.addSensor(SensorType::voltage, SensorDataType::uint16_t, "mV", voltage);
    
    auto dataVec = nodeLora.encode();

    Serial.printf("\r\nsending packet, length %d\r\n", dataVec.size());
    Radio.Send(dataVec.data(), dataVec.size());
    
    state = STATES::LOW_POWER;
    break;
  }
  case STATES::LOW_POWER:
  {
    turnOffRGB();
    lowPowerHandler();    
    delay(60000); // LowPower time
    state = STATES::READ_SENSORS;
    break;
  }
  case STATES::READ_SENSORS:
  {    
    voltage = getBatteryVoltage();
    ambientTemperature = getMedianValue<float>([]{return mlx.readAmbientTempC();});
    objectTemperature = getMedianValue<float>([]{return mlx.readObjectTempC();});
    
    Serial.print("Ambient = ");
    Serial.print(ambientTemperature);
    Serial.print("*C\tObject = ");
    Serial.print(objectTemperature);
    Serial.println("*C");

    state = STATES::TX;
    break;
  }
  default:
    break;
  }
  Radio.IrqProcess();
}

void OnTxDone()
{
  Serial.print("TX done!");
  turnOnRGB(0, 0);
}

void OnTxTimeout()
{
  Radio.Sleep();
  Serial.print("TX Timeout......");
  state = STATES::READ_SENSORS;
}
