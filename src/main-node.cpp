#include "loraSettings.h"
#include "nodeLora.h"

#include "LoRaWan_APP.h"
#include "Wire.h"
// #include "Arduino.h"

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

void OnTxDone(void);
void OnTxTimeout(void);

typedef enum
{
  LOWPOWER,
  ReadSensors,
  TX
} States_t;

States_t state;

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

  state = ReadSensors;

  initMlx();
}

void loop()
{
  switch (state)
  {
  case TX:
  {
    turnOnRGB(COLOR_SEND, 0);

    float fVoltage = voltage;    

    NodeLora sensorLora(0, firstRun ? NodeCommand::handshake : NodeCommand::state);    
    firstRun = false;

    sensorLora.addSensor(SensorType::temperature, SensorDataType::floatType, "°C", ambientTemperature);
    sensorLora.addSensor(SensorType::temperature, SensorDataType::floatType, "°C", objectTemperature);
    sensorLora.addSensor(SensorType::voltage, SensorDataType::floatType, "mV", fVoltage);
    
    auto dataVec = sensorLora.encode();

    Serial.printf("\r\nsending packet, length %d\r\n", dataVec.size());
    Radio.Send(dataVec.data(), dataVec.size());
    
    state = LOWPOWER;
    break;
  }
  case LOWPOWER:
  {
    lowPowerHandler();
    delay(100);
    turnOffRGB();
    delay(60000); // LowPower time
    state = ReadSensors;
    break;
  }
  case ReadSensors:
  {
    pinMode(VBAT_ADC_CTL, OUTPUT);
    digitalWrite(VBAT_ADC_CTL, LOW);
    voltage = analogRead(ADC) * 2;

    /*
     * Board, BoardPlus, Capsule, GPS and HalfAA variants
     * have external 10K VDD pullup resistor
     * connected to GPIO7 (USER_KEY / VBAT_ADC_CTL) pin
     */
    pinMode(VBAT_ADC_CTL, INPUT);

    ambientTemperature = mlx.readAmbientTempC();
    objectTemperature = mlx.readObjectTempC();

    Serial.print("Ambient = ");
    Serial.print(ambientTemperature);
    Serial.print("*C\tObject = ");
    Serial.print(objectTemperature);
    Serial.println("*C");

    state = TX;
    break;
  }
  default:
    break;
  }
  Radio.IrqProcess();
}

void OnTxDone(void)
{
  Serial.print("TX done!");
  turnOnRGB(0, 0);
}

void OnTxTimeout(void)
{
  Radio.Sleep();
  Serial.print("TX Timeout......");
  state = ReadSensors;
}
