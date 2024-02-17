#include <chrono>

#include "Arduino.h"

#include "debug.h"
#include "displayExt.h"
#include "loraSettings.h"
#include "mqttManager.h"
#include "nodeLora.h"
#include "wifiManagerExt.h"

DisplayExt displayExt;
WifiManagerExt wifiManagerExt(displayExt);
MqttManager mqttManager(wifiManagerExt, displayExt);

std::list<std::string> listOfSensors;
std::chrono::steady_clock::time_point lastUpdateTime;

std::chrono::steady_clock::time_point btnHoldTime;

constexpr int TIME_DURATION_FOR_BIND_SEC = 3;
constexpr int TIME_DURATION_FOR_RESET_SEC = 10;

std::array<String, 5> displayLines;

enum class STATES : uint8_t
{
  PROCESSING,
  BINDING,
  RESET,
  BUTTON_PRESSED
};
STATES g_state;

void restartESP()
{
  displayExt.printLn("Restarting...");
  delay(3000);
  ESP.restart();
}

void initLora()
{
  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  Radio.RxBoosted(0);
}

const int prgBtnPin = 0; // PRG button

void setup()
{
  Debug::setup();
  // WIFI Kit series V1 not support Vext control
  Mcu.begin();

  displayExt.setup();

  initLora();

  wifiManagerExt.setup();
  mqttManager.setup();

  pinMode(prgBtnPin, INPUT);
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  NodeLora nodeLora;

  if (!nodeLora.decode(payload, size))
  {
    debugf("Can't decode the payload\r\n");
    return;
  }

  lastUpdateTime = std::chrono::steady_clock::now();

  auto newSensorsList = mqttManager.publish(nodeLora);
  listOfSensors.insert(listOfSensors.end(), newSensorsList.begin(), newSensorsList.end());

  Radio.Sleep();
  debugf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n", payload, rssi, size);
}

void handleStateProcessing()
{
  wifiManagerExt.loop();

  if (!mqttManager.connect())
  {
    wifiManagerExt.startConfigPortal();
    restartESP();
  }

  int i = 1;
  for (auto line : listOfSensors)
  {
    displayLines[i] = line.c_str();
    i++;

    if (i >= displayLines.size())
    {
      break;
    }
  }

  while (listOfSensors.size() > displayLines.size() - 1)
  {
    listOfSensors.pop_front();
  }

  for (auto line : listOfSensors)
  {
    debugf("ss %s \r\n", line.c_str());
  }

  auto durationSinceLastUpdate = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - lastUpdateTime).count();
  displayLines[0] = (std::string("Last update ") + std::to_string(durationSinceLastUpdate) + " min ago").c_str();

  int prgBtnState = digitalRead(prgBtnPin);
  if (prgBtnState == LOW)
  {
    debugln("Button is pressed!");
    btnHoldTime = std::chrono::steady_clock::now();
    g_state = STATES::BUTTON_PRESSED;
  }

  mqttManager.loop();
  Radio.Rx(0);
  Radio.IrqProcess();

  displayExt.printLines(displayLines);
}

void handleStateButtons()
{
  int prgBtnState = digitalRead(prgBtnPin);
  auto durationSincePressed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - btnHoldTime).count();
  if (prgBtnState == HIGH)
  {
    debugln("Button is released!");

    if (durationSincePressed > TIME_DURATION_FOR_RESET_SEC)
    {
      g_state = STATES::RESET;
    }
    else if (durationSincePressed > TIME_DURATION_FOR_BIND_SEC)
    {
      g_state = STATES::BINDING;
    }
  }

  if (durationSincePressed > TIME_DURATION_FOR_RESET_SEC)
  {
    displayExt.printLn("-> Release to RESET");
  }
  else if (durationSincePressed > TIME_DURATION_FOR_BIND_SEC)
  {
    displayExt.printLn("-> Release to bind");
  }
}

void loop()
{
  switch (g_state)
  {
  case STATES::PROCESSING:
    handleStateProcessing();
    break;
  case STATES::BINDING:
    break;
  case STATES::RESET:
    wifiManagerExt.resetSettings();
    restartESP();
    break;
  case STATES::BUTTON_PRESSED:
    handleStateButtons();
    break;
  }

  delay(100);

  debug("ESP.getFreeHeap(): ");
  debugln(ESP.getFreeHeap());
}