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

        // TODO change back to _refreshRateMs
        
        if (!Raft::isTimeout(millis(), _lastLoopMs, 100))
            return;
        _lastLoopMs = millis();

        // Calculations
        uint32_t totalColsWithCharData = (_msg.length() * (_charWidthCols + _interCharBlankCols) - 1);

        // Iterate over columns in LED grid
        for (uint32_t colIdx = 0; colIdx < _gridWidth; colIdx++)
        {
            // Calculations
            uint32_t curAnimColumn = (_curAnimCount + colIdx) % (_preMsgBlankCols + totalColsWithCharData + _postMsgBlankCols);
            bool preMsgBlank = curAnimColumn < _preMsgBlankCols;
            bool postMsgBlank = curAnimColumn >= totalColsWithCharData + _preMsgBlankCols;
            uint32_t colWithinCharIdx = (curAnimColumn - _preMsgBlankCols) % (_charWidthCols + _interCharBlankCols);
            bool interCharBlank = colWithinCharIdx >= _charWidthCols;

            // LOG_I("LEDPatternScrollMsg", "colIdx %d curAnimColumn %d preMsgBlank %d postMsgBlank %d colWithinCharIdx %d interCharBlank %d", 
            //             colIdx, curAnimColumn, preMsgBlank, postMsgBlank, colWithinCharIdx, interCharBlank);
        
            // Handle display
            if (preMsgBlank || postMsgBlank || interCharBlank)
            {
                // Blank column
                for (uint32_t rowIdx = 0; rowIdx < _gridHeight; rowIdx++)
                {
                    _pixels.setRGB((_gridWidth-colIdx-1) + rowIdx*_gridWidth, 0, 0, 0);
                }
            }
            else
            {
                // Get character
                uint32_t charIdx = (curAnimColumn - _preMsgBlankCols) / (_charWidthCols + _interCharBlankCols);
                char ch = _msg.charAt(charIdx);

                // Check valid
                if (ch < LED_PATTERN_FONT_5X5_ASCII_OFFSET || ch >= LED_PATTERN_FONT_5X5_ASCII_OFFSET + LED_PATTERN_FONT_5X5_ASCII_SIZE)
                    continue;
                    
                // Get character data
                const uint8_t* pCharData = LEDPatternFont5x5[ch-LED_PATTERN_FONT_5X5_ASCII_OFFSET];
                if (!pCharData)
                    continue;
                if (colWithinCharIdx >= LED_PATTERN_FONT_5X5_FONT_WIDTH)
                    continue;
                pCharData += colWithinCharIdx;

                // Draw column
                uint8_t bitMask = 0x40;
                for (uint32_t rowIdx = 0; rowIdx < _gridHeight; rowIdx++)
                {
                    uint32_t ledIdx = (_gridWidth-colIdx-1) + rowIdx*_gridWidth;
                    _pixels.setRGB(ledIdx, (*pCharData) & bitMask ? _charColourRGB : 0);
                    bitMask >>= 1;
                }
            }
        }

        // Show pixels
        _pixels.show();
        
        // Update animation count
        _curAnimCount++;
        if (_curAnimCount >= totalColsWithCharData + _preMsgBlankCols + _postMsgBlankCols)
            _curAnimCount = 0;

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

    // Display grid
    uint32_t _gridWidth = 5;
    uint32_t _gridHeight = 5;

    // Character colour
    uint32_t _charColourRGB = 0x100000;
    
    // Message
    String _msg = "Hello World!";
};

