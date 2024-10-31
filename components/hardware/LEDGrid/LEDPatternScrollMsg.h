/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Pattern Scrolling Message
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef FEATURE_OLD_LED_GRID

#pragma once

#include "LEDPatternBase.h"
#include "Font5x5.h"
#include "RaftJson.h"

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
        RaftJson params(pParamsJson);
    }

    // Loop
    void loop() override final
    {
        // Check update time
        if (!Raft::isTimeout(millis(), _lastLoopMs, _refreshRateMs * _rateMultiple))
            return;
        _lastLoopMs = millis();

        // Check for new message
        if (isNewMsg())
        {
            // Fill array of char start indexes
            _msgCharNextColIdxs.resize(_msg.length());
            _msgCharTotalWidths = 0;
            for (uint32_t i = 0; i < _msg.length(); i++)
            {
                uint32_t ch = _msg.charAt(i);
                if (ch < Font5x5::start || ch >= Font5x5::end)
                    continue;
                _msgCharTotalWidths += Font5x5::font[ch - Font5x5::start][0] + _interCharBlankCols;
                _msgCharNextColIdxs[i] = _msgCharTotalWidths;
                // LOG_I("LEDPatternScrolling", "chIdx %d ch %02x nextCharStartIdx %d totalWidth %d fontCharWidth %d", 
                //             i, ch, _msgCharNextColIdxs[i], _msgCharTotalWidths, Font5x5::font[ch - Font5x5::start][0]);
            }

            // LOG_I("LEDPatternScrolling", "New message %s totalWidth %d numStartIdxs %d", 
            //             _msg.c_str(), _msgCharTotalWidths, _msgCharNextColIdxs.size());
        }

        // Iterate over columns in LED grid
        uint32_t totalCols = _preMsgBlankCols + _msgCharTotalWidths + _postMsgBlankCols;
        for (uint32_t colIdx = 0; colIdx < _gridWidth; colIdx++)
        {
            // Column calculations
            uint32_t curAnimColumn = (_curAnimCount + colIdx) % totalCols;
            bool preMsgBlank = curAnimColumn < _preMsgBlankCols;
            bool postMsgBlank = curAnimColumn >= _msgCharTotalWidths + _preMsgBlankCols;
            uint32_t curMsgColumn = curAnimColumn - _preMsgBlankCols;

            // LOG_I("LEDPatternScrolling", "colIdx %d curAnimColumn %d preMsgBlank %d postMsgBlank %d curMsgColumn %d", 
            //             colIdx, curAnimColumn, preMsgBlank, postMsgBlank, curMsgColumn);
            
            // Character calculations
            bool interCharBlank = false;
            uint32_t colWithinChar = curMsgColumn;
            uint32_t charToDispIdx = 0;
            if (!preMsgBlank && !postMsgBlank)
            {
                // Establish character
                for (charToDispIdx = 0; charToDispIdx < _msgCharNextColIdxs.size(); charToDispIdx++)
                {
                    if (curMsgColumn < _msgCharNextColIdxs[charToDispIdx])
                    {
                        colWithinChar = curMsgColumn - ((charToDispIdx == 0) ? 0 : _msgCharNextColIdxs[charToDispIdx-1]);
                        interCharBlank = curMsgColumn >= _msgCharNextColIdxs[charToDispIdx] - _interCharBlankCols;
                        break;
                    }
                }
            }

            // LOG_I("LEDPatternScrolling", "colIdx %d curAnimColumn %d preMsgBlank %d postMsgBlank %d colWithinChar %d interCharBlank %d charToDispIdx %d",
            //             colIdx, curAnimColumn, preMsgBlank, postMsgBlank, colWithinChar, interCharBlank, charToDispIdx);
        
            // Handle display
            if (preMsgBlank || postMsgBlank || interCharBlank)
            {
                // Blank column
                for (uint32_t rowIdx = 0; rowIdx < _gridHeight; rowIdx++)
                {
                    _pixels.setRGB((_gridWidth - colIdx - 1) + rowIdx*_gridWidth, 0, 0, 0);
                }
            }
            else
            {
                // Get character
                char ch = _msg.charAt(charToDispIdx);

                // Check valid
                if (ch < Font5x5::start || ch >= Font5x5::end)
                    continue;
                    
                // Get character data
                const uint8_t* pCharData = Font5x5::font[ch - Font5x5::start] + 1;
                if (!pCharData)
                    continue;

                // Set the bit mask based on the column
                uint8_t bitMask = 0x80 >> colWithinChar;

                // Draw column
                for (uint32_t rowIdx = 0; rowIdx < _gridHeight; rowIdx++)
                {
                    if (rowIdx >= Font5x5::height)
                        break;
                    uint32_t ledIdx = (_gridWidth - colIdx - 1) + rowIdx*_gridWidth;
                    _pixels.setRGB(ledIdx, pCharData[rowIdx] & bitMask ? _charColourRGB : 0);
                }
            }
        }

        // Show pixels
        _pixels.show();
        
        // Update animation count
        _curAnimCount++;
        if (_curAnimCount >= totalCols)
            _curAnimCount = 0;
    }

private:
    // Rate
    uint32_t _lastLoopMs = 0;
    uint32_t _rateMultiple = 3;

    // Animation state
    uint32_t _curAnimCount = 0;

    // Font
    uint32_t _preMsgBlankCols = 2;
    uint32_t _postMsgBlankCols = 2;
    static const uint32_t _interCharBlankCols = 1;

    // Display grid
    uint32_t _gridWidth = 5;
    uint32_t _gridHeight = 5;

    // Character colour
    uint32_t _charColourRGB = 0x100000;
    
    // Message
    String _msg = "I Love You XXX";
    std::vector<uint16_t> _msgCharNextColIdxs;
    uint32_t _msgCharTotalWidths = 0;

    // Helpers
    bool isNewMsg()
    {

        // TODO remove
        return true;


        // if (!_pNamedValueProvider)
        //     return false;
        // // Check for new message
        // String newMsg = _pNamedValueProvider->getString("msg", "");
        // if (newMsg != _msg)
        // {
        //     _msg = newMsg;
        //     return true;
        // }
        // return false;
    }
};

#endif // FEATURE_OLD_LED_GRID