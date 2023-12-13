/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Pattern Scrolling Message
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LEDPatternBase.h"
#include "LEDPatternFont5x5.h"
#include <JSONParams.h>

class LEDPatternScrollMsg : public LEDPatternBase
{
public:
    LEDPatternScrollMsg(NamedValueProvider* pNamedValueProvider, LEDPixels& pixels) :
        LEDPatternBase(pNamedValueProvider, pixels)
    {
    }
    virtual ~LEDPatternScrollMsg()
    {
    }

    // Build function for factory
    static LEDPatternBase* build(NamedValueProvider* pNamedValueProvider, LEDPixels& pixels)
    {
        return new LEDPatternScrollMsg(pNamedValueProvider, pixels);
    }

    // Setup
    virtual void setup(const char* pParamsJson = nullptr) override final
    {
        // Check setup valid
        if (!pParamsJson)
            return;

        // Get JSON
        JSONParams params(pParamsJson);

        // 


    }

    // Service
    void service() override final
    {
        // Check update time
        if (!Raft::isTimeout(millis(), _lastLoopMs, _refreshRateMs))
            return;
        _lastLoopMs = millis();

        // // Calculations
        // uint32_t totalColsWithCharData = (_msg.length() * (_charWidthCols + _interCharBlankCols) - 1);
        // bool preMsgBlank = _curAnimCount < _preMsgBlankCols;
        // bool postMsgBlank = _curAnimCount >= totalColsWithCharData + _preMsgBlankCols;
        // uint32_t charIdx = (_curAnimCount - _preMsgBlankCols) / (_charWidthCols + _interCharBlankCols);
        // uint32_t colWithinCharIdx = (_curAnimCount - _preMsgBlankCols) % (_charWidthCols + _interCharBlankCols);
        // bool interCharBlank = colWithinCharIdx >= _charWidthCols;
        
        // // Handle display


        // if !preMsgBlank
        // {
        //     bool postMsgBlank = _curAnimCount >= (_msg.length() + PRE_MSG_BLANK_LINES + POST_MSG_BLANK_LINES);
        //     if !postMsgBlank
        //     {
        //         // Get character
        // uint32_t charIdx = _curAnimCount / INTER_CHAR_BLANK_LINES;
        

        // // if (_curState)
    }

private:
    // Rate
    uint32_t _lastLoopMs = 0;

    // Animation state
    uint32_t _curAnimCount = 0;

    // Font
    uint32_t _preMsgBlankCols = 2;
    uint32_t _postMsgBlankCols = 2;
    uint32_t _charWidthCols = 5;
    static const uint32_t _interCharBlankCols = 1;
    
    // Message
    String _msg;
};

