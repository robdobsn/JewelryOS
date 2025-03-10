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
#include "ConfigPinMap.h"
#include "driver/gpio.h"

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

void LEDHeart::setup(const RaftJsonIF& config)
{
    // Display brightness
    _displayBrightnessPC = config.getInt("brightnessPC", 100);

    // Get time between animation steps
    _animStepTimeUs = config.getInt("animStepTimeUs", ANIM_STEP_TIME_US_DEFAULT);

    // Get LED pins
    std::vector<String> ledPinStrs;
    config.getArrayElems("ledPins", ledPinStrs);
    if (ledPinStrs.size() == 0)
    {
        LOG_I(MODULE_PREFIX, "setup no LED pins defined");
        return;
    }

    // Extract LED pins
    _ledPins.clear();
    for (int ledIdx = 0; ledIdx < ledPinStrs.size(); ledIdx++)
    {
        int ledPin = ConfigPinMap::getPinFromName(ledPinStrs[ledIdx].c_str());
        if (ledPin > 0)
            _ledPins.push_back(ledPin);
    }

    // Get LED intensity factors
    std::vector<String> ledIntensityFactorStrs;
    config.getArrayElems("ledIntensityFactors", ledIntensityFactorStrs);

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
    _ledActiveLevel = config.getBool("ledActiveLevel", false);

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
    LOG_I(MODULE_PREFIX, "setup OK numLEDs %d activeLevel %d pin(intensity): %s", _ledPins.size(), _ledActiveLevel, ledPinsStr.c_str());
#else
    LOG_I(MODULE_PREFIX, "setup OK numLEDs %d activeLevel %d", _ledPins.size(), _ledActiveLevel);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loop
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDHeart::loop()
{
    // Turn off any LEDs that need to be turned off
    for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    {
        if (_animationOffAfterUs[ledIdx] > 0)
        {
            if (Raft::isTimeout(micros(), _lastAnimTimeUs, _animationOffAfterUs[ledIdx]))
            {
                gpio_hold_dis((gpio_num_t)_ledPins[ledIdx]);
                digitalWrite(_ledPins[ledIdx], !_ledActiveLevel);
                gpio_hold_en((gpio_num_t)_ledPins[ledIdx]);
                _animationOffAfterUs[ledIdx] = 0;
                // LOG_I(MODULE_PREFIX, "loop setLedOff ledIdx %d", ledIdx);
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

        // Set last animation step time
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
                int animOffTimeUs = _animationStepLevels[animStepIdx] * _ledIntensityFactors[ledIdx] * _displayBrightnessPC;

                // Set LED state
#ifdef FEATURE_HEART_ANIMATIONS
                gpio_hold_dis((gpio_num_t)_ledPins[ledIdx]);
                digitalWrite(_ledPins[ledIdx], animOffTimeUs != 0 ? _ledActiveLevel : !_ledActiveLevel);
                gpio_hold_en((gpio_num_t)_ledPins[ledIdx]);
#endif
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
        // gpio_hold_en((gpio_num_t)_ledPins[ledIdx]);
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
    // LOG_I(MODULE_PREFIX, "getTimeToNextAnimStepMs nextAnimStepTimeMs %d animOffTimeUs %d timeUs %d", 
    //         _nextAnimStepAfterUs, (int)animOffTimeUs, (int)timeUs);
    return timeUs;
}
