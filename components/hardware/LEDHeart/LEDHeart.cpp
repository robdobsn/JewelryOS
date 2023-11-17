/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Heart Display
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LEDHeart.h"
#include "Logger.h"
#include "RaftUtils.h"

static const char* MODULE_PREFIX = "LEDHeart: ";

#define DEBUG_LED_HEART_PINS
// #define USE_FIXED_TIMER_FOR_LED_BRIGHTNESS

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LEDHeart::LEDHeart()
{
}

LEDHeart::~LEDHeart()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDHeart::setup(ConfigBase& config, const char* pConfigPrefix)
{
    // Get LED pins
    std::vector<String> ledPinStrs;
    config.getArrayElems("ledPins", ledPinStrs, pConfigPrefix);
    if (ledPinStrs.size() == 0)
    {
        LOG_I(MODULE_PREFIX, "setup no LED pins defined");
        return;
    }

    // Extract LED pins
    _ledPins.clear();
    for (int ledIdx = 0; ledIdx < ledPinStrs.size(); ledIdx++)
    {
        int ledPin = ledPinStrs[ledIdx].toInt();
        if (ledPin > 0)
            _ledPins.push_back(ledPin);
    }

    // Get LED intensity factors
    std::vector<String> ledIntensityFactorStrs;
    config.getArrayElems("ledIntensityFactors", ledIntensityFactorStrs, pConfigPrefix);

    // Set intensity factors for each defined LED
    _ledIntensityFactors.resize(_ledPins.size());
    for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    {
        if (ledIdx < ledIntensityFactorStrs.size())
            _ledIntensityFactors[ledIdx] = ledIntensityFactorStrs[ledIdx].toInt();
        else
            _ledIntensityFactors[ledIdx] = 1;
    }

    // Set size of animation off time array
    _animationOffAfterUs.resize(_ledPins.size());
    for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
        _animationOffAfterUs[ledIdx] = 0;

    // Get active level for LED pins
    _ledActiveLevel = config.getBool("ledActiveLevel", false, pConfigPrefix);

    // Set LED pins to output and off
    for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    {
        pinMode(_ledPins[ledIdx], OUTPUT);
        digitalWrite(_ledPins[ledIdx], !_ledActiveLevel);
    }

    // Set animation step count
    _animationCount = _animationStepLevels.size() + _ledPins.size() - 1;

    // Log
#ifdef DEBUG_LED_HEART_PINS
    String ledPinsStr;
    for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    {
        if (ledIdx > 0)
            ledPinsStr += ",";
        ledPinsStr += String(_ledPins[ledIdx]) + "(" + String(_ledIntensityFactors[ledIdx]) + ")";
    }
    LOG_I(MODULE_PREFIX, "setup OK numLEDs %d activeLevel %d pin(intesity): %s", _ledPins.size(), _ledActiveLevel, ledPinsStr.c_str());
#else
    LOG_I(MODULE_PREFIX, "setup OK numLEDs %d activeLevel %d", _ledPins.size(), _ledActiveLevel);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Service
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDHeart::service()
{
    // Turn off any LEDs that need to be turned off
    for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    {
        if (_animationOffAfterUs[ledIdx] > 0)
        {
            if (Raft::isTimeout(micros(), _lastAnimTimeUs, _animationOffAfterUs[ledIdx]))
            {
                digitalWrite(_ledPins[ledIdx], !_ledActiveLevel);
                _animationOffAfterUs[ledIdx] = 0;
                // LOG_I(MODULE_PREFIX, "service setLedOff ledIdx %d", ledIdx);
            }
        }
    }

    // Check if time for next step in sequence
    if (Raft::isTimeout(micros(), _lastAnimTimeUs, _nextAnimStepAfterUs))
    {
        // Handle animation step
        handleAnimationStep();

        // Update animation step
        _animationStepNum++;
        if (_animationStepNum >= _animationCount)
        {
            _animationStepNum = 0;
            _nextAnimStepAfterUs = UINT32_MAX;
        }

        // Set next animation step time
        _lastAnimTimeUs = micros();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handle animation step
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDHeart::handleAnimationStep()
{
    // Set LEDs
    for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    {
        // Use animation step to determine LED state
        int animStepIdx = (int)_animationStepNum - ledIdx;
        if (animStepIdx >= 0)
        {
            if (animStepIdx < _animationStepLevels.size())
            {
                // Set animation off after time
                int animOffTimeUs = _animationStepLevels[animStepIdx] * _ledIntensityFactors[ledIdx] * _displayBrightnessLevel;

                // Set LED state
                digitalWrite(_ledPins[ledIdx], animOffTimeUs != 0 ? _ledActiveLevel : !_ledActiveLevel);
                _animationOffAfterUs[ledIdx] = animOffTimeUs;

                // LOG_I(MODULE_PREFIX, "handleAnimationStep setLedOn ledIdx %d animStepIdx %d offTimeMs %d", 
                //         ledIdx, animStepIdx, _animationOffAfterMs[ledIdx]);
            }
        }
    }

#ifdef USE_FIXED_TIMER_FOR_LED_BRIGHTNESS
    delay(1);
    for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    {
        digitalWrite(_ledPins[ledIdx], !_ledActiveLevel);
    }

#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get time to next animation step
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t LEDHeart::getTimeToNextAnimStepUs()
{
    // Check if any LEDs need to be turned off
    uint32_t animOffTimeUs = UINT32_MAX;
    for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    {
        if (_animationOffAfterUs[ledIdx] > 0)
        {
            if (_animationOffAfterUs[ledIdx] < animOffTimeUs)
                animOffTimeUs = _animationOffAfterUs[ledIdx];
        }
    }

    uint32_t timeUs = _nextAnimStepAfterUs < animOffTimeUs ? _nextAnimStepAfterUs : animOffTimeUs;
    // LOG_I(MODULE_PREFIX, "getTimeToNextAnimStepMs nextAnimStepTimeMs %d animOffTimeMs %d timeMs %d", 
    //         _nextAnimStepAfterUs, animOffTimeMs, timeMs);
    return timeUs;
}
