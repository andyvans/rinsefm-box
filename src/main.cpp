#include <Arduino.h>
#include <WiFi.h>
#include "AudioOut.h"
#include "DeviceControls.h"
#include "ChannelManager.h"
#include "_Secrets.h"

// Configuration URL - change this to your config file location
#define CONFIG_URL "https://raw.githubusercontent.com/andyvans/ultimate-radio/main/radio-config.txt"

AudioOut* audioOut;
DeviceControls* deviceControls;
RadioConfig* radioConfig;

TaskHandle_t DeviceTask = NULL;

void ProcessDevices(void* parameter);

void setup()
{
  Serial.begin(115200);
  delay(200);

  Serial.println("\n\n=== RinseFM Box Starting ===");

  // Initialize WiFi with timeout (10 seconds)
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int wifiTimeout = 10000; // 10 seconds
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < wifiTimeout)
  {
    delay(100);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi connected!");
  }
  else
  {
    Serial.println("\nWiFi connection timeout - continuing without network");
  }

  radioConfig = nullptr; // ChannelManager::LoadChannels(WIFI_SSID, WIFI_PASSWORD, CONFIG_URL);
  if (radioConfig == nullptr)
  {
    Serial.println("Using default channels");
    radioConfig = ChannelManager::GetDefaultChannels();
  }

  audioOut = new AudioOut();
  audioOut->Setup(radioConfig->channels, radioConfig->channelCount, radioConfig->defaultChannel);

  deviceControls = new DeviceControls();
  deviceControls->Setup(audioOut);

  // Create task for device controls
  Serial.println("Creating tasks");
  xTaskCreatePinnedToCore(
    ProcessDevices,
    "Device",
    4096, // Reduced for just wled and device controls
    NULL,
    1,
    &DeviceTask,
    0); // Core 0 (shared with WiFi & system tasks)  
}

void loop()
{
  // Only process audio in the main loop (core 1)
  if (audioOut != nullptr) audioOut->Tick();
}

// Task to process device controls (core 0)
void ProcessDevices(void* parameter)
{
  for (;;)
  {
    if (deviceControls != nullptr)
    {
      deviceControls->Tick();
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}