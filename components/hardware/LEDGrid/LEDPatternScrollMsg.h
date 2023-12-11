/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Pattern Scrolling Message
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LEDPatternBase.h"

class LEDPatternScrollMsg : public LEDPatternBase
{
public:
    LEDPatternScrollMsg(NamedValueProvider* pNamedValueProvider, std::vector<LEDPixel>& pixels, ESP32RMTLedStrip& ledStrip) :
        LEDPatternBase(pNamedValueProvider, pixels, ledStrip)
    {
    }
    virtual ~LEDPatternScrollMsg()
    {
    }

    // Build function for factory
    static LEDPatternBase* build(NamedValueProvider* pNamedValueProvider, std::vector<LEDPixel>& pixels, ESP32RMTLedStrip& ledStrip)
    {
        return new LEDPatternScrollMsg(pNamedValueProvider, pixels, ledStrip);
    }

    // Setup
    void setup()
    {
    }

    // Service
    void service()
    {
        // Check update time
        if (!Raft::isTimeout(millis(), _lastLoopMs, _refreshRateMs))
            return;
        _lastLoopMs = millis();

        // 

        if (_curState)
        {
            for (int pixIdx = _curIter; pixIdx < _pixels.size(); pixIdx += 3)
            {
                uint16_t hue = pixIdx * 360 / _pixels.size() + _curHue;
                _pixels[pixIdx].fromRGB(LEDPixHSV::toRGB(hue, 100, 10), LEDPixel::RGB, 1.0f);
            }
            // Show pixels
            _ledStrip.showPixels(_pixels);
        }
        else
        {
            for (auto& pix : _pixels)
                pix.clear();
            _ledStrip.showPixels(_pixels);
            _curIter = (_curIter + 1) % 3;
            if (_curIter == 0)
            {
                _curHue += 60;
            }
        }
        _curState = !_curState;
    }

private:
    // State
    uint32_t _lastLoopMs = 0;
    bool _curState = false;
    uint32_t _curIter = 0;
    uint32_t _curHue = 0;

    // Font
    static const uint32_t PRE_MSG_BLANK_LINES = 2;
    static const uint32_t POST_MSG_BLANK_LINES = 2;
    static const uint32_t POST_CHAR_BLANK_LINES = 1;
    
    // Message
    String _msg;
};

