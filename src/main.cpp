#include <Arduino.h>
#include <WiFiManager.h>
#include "AudioOut.h"
#include "DeviceControls.h"
#include "ChannelManager.h"

// Configuration URL - change this to your config file location
#define CONFIG_URL "https://raw.githubusercontent.com/andyvans/rinsefm-box/main/radio-config.txt"

AudioOut* audioOut;
DeviceControls* deviceControls;
RadioConfig* radioConfig;

TaskHandle_t DeviceTask = NULL;

void ProcessDevices(void* parameter);

void setup()
{
  Serial.begin(115200);
  delay(200);

  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("PSRAM size: %u bytes\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());

  WiFi.setHostname("UltimateRadio");

  // Connect to WiFi using WiFiManager captive portal
  WiFiManager wm;
  wm.setConfigPortalTimeout(180); // 3 minute timeout
  Serial.println("Connecting to WiFi...");
  if (wm.autoConnect("RinseFmBox-Setup"))
  {
    Serial.println("WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("WiFi connection failed - restarting...");
    ESP.restart();
  }

  // AAC support requires PSRAM due to the larger buffers
  bool supportAac = ESP.getPsramSize() > 0;
  if (!supportAac)
    Serial.println("No PSRAM detected - AAC support disabled");
    
  Serial.println("=== RinseFM Box Starting ===");
  
  radioConfig = ChannelManager::LoadChannels(CONFIG_URL);
  if (radioConfig == nullptr)
  {
    Serial.println("Using default channels");
    radioConfig = ChannelManager::GetDefaultChannels();
  }

  audioOut = new AudioOut(supportAac);
  audioOut->Setup(radioConfig);

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