/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Heart Display
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <ConfigBase.h>

class LEDHeart
{
public:
    // Constructor / Destructor
    LEDHeart();
    ~LEDHeart();

    // Setup
    void setup(const ConfigBase& config, const char* pConfigPrefix);

    // Service
    void service();

    // Get time to next animation step
    uint32_t getTimeToNextAnimStepUs();

    // Start pulse animation
    void startPulseAnimation()
    {
        _nextAnimStepAfterUs = 50000;
        _animationStepNum = 0;
    }

private:

    // Brightness level
    uint32_t _displayBrightnessLevel = 100;

    // LED pins and intensity factors
    std::vector<int> _ledPins;
    std::vector<int> _ledIntensityFactors;

    // Active level
    bool _ledActiveLevel = false;

    // Timer for animation
    uint64_t _lastAnimTimeUs = 0;

    // Next animation step time
    uint32_t _nextAnimStepAfterUs = 50000;

    // Animation step number
    uint32_t _animationStepNum = 0;

    // Animation off times for LEDs
    std::vector<uint32_t> _animationOffAfterUs;

    // Animation step levels
    static const uint8_t OFF_LEVEL = 0;
    static const uint8_t LOW_LEVEL = 1;
    static const uint8_t MID_LEVEL = 2;
    static const uint8_t HIGH_LEVEL = 3;
    std::vector<uint8_t> _animationStepLevels = 
    {
        LOW_LEVEL, MID_LEVEL, HIGH_LEVEL, MID_LEVEL, LOW_LEVEL, OFF_LEVEL
    };

    // Animation level count
    uint32_t _animationCount = 0;

    // Helpers
    void handleAnimationStep();
};
