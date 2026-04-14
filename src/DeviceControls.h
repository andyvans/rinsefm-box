#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "OneRotaryEncoder.h"
#include "constants.h"
#include "AudioOut.h"

class DeviceControls
{
public:
    DeviceControls();
    void Setup(AudioOut* audio);
    void Tick();

private:
    int _currentChannel;
    int _pendingChannel;
    unsigned long _lastPositionChangeTime;
    bool _hasPendingChange;
    bool _displayInitialized;
    OneRotaryEncoder* _encoder;
    AudioOut* _audioOut;
    U8G2_SH1106_128X64_NONAME_F_HW_I2C* _display;

    int ChannelChangeDelayMs = 750;

    void UpdateDisplay(int channel, const char* name);
};
