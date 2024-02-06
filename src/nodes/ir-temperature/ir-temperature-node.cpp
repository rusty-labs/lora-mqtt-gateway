#include <functional>

#include <Adafruit_MLX90614.h>
#undef abs

#include "LoRaWan_APP.h"
#include "Wire.h"

#include "debug.h"
#include "loraSettings.h"
#include "medianFilter.h"
#include "nodeLora.h"
#include "utils.h"


Adafruit_MLX90614 g_Mlx = Adafruit_MLX90614();
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
  SEND_DATA
};
STATES g_state;

namespace SENSORS_DATA
{
  uint16_t voltage = 0;
  float objectTemperature = 0;
  float ambientTemperature = 0;
}
bool g_sendDiscovery = true;
constexpr uint32_t g_updateIntervalMs = 60000;

const int userBtnPin = USER_KEY; // USER button

void initMlx()
{
  if (!g_Mlx.begin())
  {
    debugln("Error connecting to MLX sensor. Check wiring.");
    while (1)
    {
      delay(1000);
    }
  };

  debug("Emissivity = ");
  debugln(g_Mlx.readEmissivity());
  debugln("================================================");
}

void setup()
{
  Debug::setup();

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  g_state = STATES::READ_SENSORS;

  initMlx();

  pinMode(userBtnPin, INPUT);
}

template <class T>
T getMedianValue(std::function<T()> getValueFunction)
{
  MedianFilter<T, 5> medianFilter;
  for (uint32_t i = 0; i < medianFilter.size(); ++i)
  {
    medianFilter.set(i, getValueFunction());
  }
  return medianFilter.getMedian();
}

void loop()
{
  switch (g_state)
  {
  case STATES::SEND_DATA:
  {
    turnOnRGB(COLOR_SEND, 0);

    NodeLora nodeLora(0, g_sendDiscovery ? NodeCommand::discovery : NodeCommand::state);
    g_sendDiscovery = false;

    nodeLora.addSensor(SensorType::temperature, SensorDataType::floatType, "°C", SENSORS_DATA::ambientTemperature);
    nodeLora.addSensor(SensorType::temperature, SensorDataType::floatType, "°C", SENSORS_DATA::objectTemperature);
    nodeLora.addSensor(SensorType::voltage, SensorDataType::uint16_t, "mV", SENSORS_DATA::voltage);

    auto dataVec = nodeLora.encode();

    debugf("\r\nsending packet, length %d\r\n", dataVec.size());
    Radio.Send(dataVec.data(), dataVec.size());

    g_state = STATES::LOW_POWER;
    break;
  }
  case STATES::LOW_POWER:
  {
    lowPowerHandler(); // LowPower time
    delay(getSpreadDelay(g_updateIntervalMs));
    g_state = STATES::READ_SENSORS;
    break;
  }
  case STATES::READ_SENSORS:
  {
    SENSORS_DATA::voltage = getBatteryVoltage();
    SENSORS_DATA::ambientTemperature = getMedianValue<float>([]
                                                             { return g_Mlx.readAmbientTempC(); });
    SENSORS_DATA::objectTemperature = getMedianValue<float>([]
                                                            { return g_Mlx.readObjectTempC(); });

    debug("Ambient = ");
    debug(SENSORS_DATA::ambientTemperature);
    debug("*C\tObject = ");
    debug(SENSORS_DATA::objectTemperature);
    debugln("*C");

    g_state = STATES::SEND_DATA;
    break;
  }
  default:
    break;
  }

  /*
    delay(100);
    int userBtnState = digitalRead(userBtnPin);

    if (userBtnState == LOW)
    {
      debugln("Button is pressed!");
      // Perform actions when the button is pressed
    }
  */

  Radio.IrqProcess();
}

void OnTxDone()
{
  debug("TX done!");
  Radio.Sleep();
  turnOffRGB();
}

void OnTxTimeout()
{
  debug("TX Timeout......");
  Radio.Sleep();
  g_state = STATES::READ_SENSORS;
}
