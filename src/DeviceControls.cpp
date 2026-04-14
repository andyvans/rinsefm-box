#include "DeviceControls.h"
#include "AudioOut.h"

DeviceControls::DeviceControls() :
    _audioOut(nullptr),
    _currentChannel(0),
    _pendingChannel(-1),
    _lastPositionChangeTime(0),
    _hasPendingChange(false),
    _displayInitialized(false),
    _encoder(nullptr),
    _display(nullptr)
{
}

void DeviceControls::Setup(AudioOut* audioOut)
{
    _audioOut = audioOut;

    Serial.println("=== Setting up DeviceControls ===");

    // Max value is maxChannels - 1 since we're using 0-based indexing
    _currentChannel = _audioOut != nullptr ? _audioOut->GetCurrentChannel() : 0;
    int maxChannel = _audioOut != nullptr ? _audioOut->GetChannelCount() - 1 : 0;

    _encoder = new OneRotaryEncoder(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_PIN_SWITCH);
    _encoder->SetRange(0, maxChannel, 4, _currentChannel);

    Serial.println("Setting up OLED display...");
    _display = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);
    _display->begin();
    _displayInitialized = true;

    // Start first radio channel by default
    Serial.print("Starting initial channel: ");
    Serial.println(_currentChannel);
    _audioOut->Start(_currentChannel);
    UpdateDisplay(_currentChannel, _audioOut->GetChannelName(_currentChannel));
}

void DeviceControls::Tick()
{
    if (_encoder == nullptr) return;
    if (_audioOut == nullptr) return;

    _encoder->Tick();

    // Check for position changes
    EncoderPositionState posState = _encoder->GetPosition();
    if (posState.hasNewPosition && posState.position != _pendingChannel)
    {
        // New position detected - update pending channel and reset timer
        _pendingChannel = posState.position;
        _lastPositionChangeTime = millis();
        _hasPendingChange = true;
        UpdateDisplay(_pendingChannel, _audioOut->GetChannelName(_pendingChannel));
    }

    // Debounce check if we should apply the pending channel change
    if (_hasPendingChange && (millis() - _lastPositionChangeTime >= ChannelChangeDelayMs))
    {
        _currentChannel = _pendingChannel;
        _hasPendingChange = false;
        _audioOut->Start(_currentChannel);
        UpdateDisplay(_currentChannel, _audioOut->GetChannelName(_currentChannel));
    }

    // Check for button presses
    EncoderSwitchState switchState = _encoder->GetSwitchState();
    if (switchState.hasNewState)
    {
        switch (switchState.state)
        {
        case EncoderSwitchPress::Clicked:
            Serial.println("Encoder button clicked");
            break;
        case EncoderSwitchPress::DoubleClicked:
            Serial.println("Encoder button double-clicked");
            break;
        case EncoderSwitchPress::LongPressed:
            Serial.println("Encoder button long-pressed");
            break;
        default:
            break;
        }
    }
}

void DeviceControls::UpdateDisplay(int channel, const char* name)
{
    if (_display == nullptr || !_displayInitialized) return;

    const bool isPendingChannel = _hasPendingChange && (channel == _pendingChannel);

    _display->clearBuffer();
    _display->setFont(u8g2_font_6x10_tf);

    // Fixed title row
    _display->drawStr(0, 10, "RinseFM Box");

    // Separator line
    _display->drawHLine(0, 14, 128);

    // Channel name text wrapped across lines with channel number on the left
    if (name != nullptr)
    {
        const int charsPerLine = 21;
        const int firstLineX = 16;
        const int firstLineChars = 18;

        char channelNum[6];
        snprintf(channelNum, sizeof(channelNum), "%d.", channel + 1);
        _display->drawStr(0, 26, channelNum);

        int nameLen = strlen(name);
        int pos = 0;

        // First line starts to the right of the channel number.
        if (nameLen > 0)
        {
            char firstSegment[19];
            int firstLen = min(firstLineChars, nameLen);
            strncpy(firstSegment, name, firstLen);
            firstSegment[firstLen] = '\0';
            _display->drawStr(firstLineX, 26, firstSegment);
            pos += firstLen;
        }

        int y = 38;
        while (pos < nameLen && y <= 62)
        {
            char segment[22];
            int segLen = min(charsPerLine, nameLen - pos);
            strncpy(segment, name + pos, segLen);
            segment[segLen] = '\0';
            _display->drawStr(0, y, segment);
            pos += segLen;
            y += 12;
        }
    }

    _display->sendBuffer();
}
