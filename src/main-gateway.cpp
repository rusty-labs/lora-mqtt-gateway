#include <WiFi.h>
#include <PubSubClient.h>
#include <set>
#include <chrono>

#include "Arduino.h"

#include "HT_SSD1306Wire.h"
#include "nodeMqtt.h"
#include "nodeLora.h"
#include "loraSettings.h"

// Check if secrets.h exists
#ifdef __has_include
#if __has_include("secrets.h")
#include "secrets.h"
#elif __has_include("secrets_example.h")
#include "secrets_example.h"
#else
#error "No secrets file found"
#endif
#else
#error "Compiler does not support __has_include"
#endif

const char *mqttServer = "homeassistant";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

std::set<uint8_t> sensorSet;
std::list<std::string> listOfSensors;

std::chrono::steady_clock::time_point lastUpdateTime;

void displayMcuInit();
void displaySendReceive();

std::array<String, 5> displayLines = {"...", "...", "...", "...", "..."};

extern SSD1306Wire display;

void messageLog(const char *msg)
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, msg);
  display.display();
}

void MqttReceiverCallback(char *topic, byte *inFrame, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  std::string messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)inFrame[i]);
    messageTemp += (char)inFrame[i];
  }
  Serial.println();

  if (std::string(topic) == std::string("homeassistant/status"))
  {
    if (messageTemp == "online")
    {
      // send discovery if home assistant was rebooted
      sensorSet.clear();
    }
  }
}

void statusDisplay()
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  for (int i = 0; i < displayLines.size(); i++)
  {
    display.drawString(1, 12 * i, displayLines[i]);
  }
  display.display();
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
}

void initMqtt()
{
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(MqttReceiverCallback);
  mqttClient.setBufferSize(2048);
}

bool connectMqtt()
{
  if (mqttClient.connected())
  {
    return true;
  }

  int attempts = 0;

  while (!mqttClient.connected() && attempts < 10)
  {
    messageLog("Connecting to MQTT...");
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("ESP32Client", MQTT_USER, MQTT_PASSWORD))
    {
      messageLog("Connected to MQTT... ");
      Serial.println("connected");

      mqttClient.subscribe("homeassistant/status");
      return true;
    }
    else
    {
      messageLog("Failed to connect to MQTT");
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }

    ++attempts;
  }

  return false;
}

void initDisplay()
{
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);  
  display.clear();
  display.drawString(0, 0, "Hello!");
  display.display();
  delay(1000);
}

void setup()
{
  Serial.begin(115200);
  // WIFI Kit series V1 not support Vext control
  Mcu.begin();

  initDisplay();
  initLora();
  initMqtt();
}

bool connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }

  messageLog("Connecting to WiFi");

  Serial.println("Connecting to WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10)
  {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    messageLog("Connected to WiFi");
    Serial.println("Connected to WiFi");
    Serial.print(" IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    return true;
  }
  else
  {
    messageLog("WiFi Failed");
    Serial.println("Failed to connect to WiFi. Please check your credentials.");
  }
  // ESP.restart();
  return false;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  NodeLora nodeLora;

  if (!nodeLora.decode(payload, size))
  {
    Serial.printf("Can't decode the payload\r\n");
    return;
  }

  lastUpdateTime = std::chrono::steady_clock::now();

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
  listOfSensors.insert(listOfSensors.end(), newSensorsList.begin(), newSensorsList.end());

  Radio.Sleep();
  Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n", payload, rssi, size);
}

void loop()
{
  if (!connectWiFi())
  {
    delay(1000);
    return;
  }
  else
  {
    displayLines[0] = "WiFi OK, RSSI :" + String(WiFi.RSSI());
  }

  if (!connectMqtt())
  {
    delay(1000);
    return;
  }
  else
  {
    displayLines[0] = "MQTT connected";
  }

  int i = 1;
  for (auto line : listOfSensors)
  {
    displayLines[i] = line.c_str();
    i++;

    if (i >= displayLines.size() )
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
    Serial.printf("ss %s \r\n", line.c_str());
  }

  auto durationSinceLastUpdate = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - lastUpdateTime).count();
  displayLines[0] = (std::string("Last update ") + std::to_string(durationSinceLastUpdate) + " min ago").c_str();

  mqttClient.loop();
  Radio.Rx(0);
  Radio.IrqProcess();

  statusDisplay();

  delay(1000);
}