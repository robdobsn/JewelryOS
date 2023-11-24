/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Grid Display
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LEDGrid.h"
#include "Logger.h"
#include "RaftUtils.h"
#include "ConfigPinMap.h"

static const char* MODULE_PREFIX = "LEDGrid";

#define HOLD_PIXEL_POWER_PIN_DURING_SLEEP

#define DEBUG_LED_GRID_SETUP
// #define USE_FIXED_TIMER_FOR_LED_BRIGHTNESS

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LEDGrid::LEDGrid()
{
}

LEDGrid::~LEDGrid()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDGrid::setup(ConfigBase& config, const char* pConfigPrefix)
{
    // Get the pixel power control pin
    String pixelPowerPin = config.getString("pixelPowerPin", "", pConfigPrefix);
    _pixelPowerPin = ConfigPinMap::getPinFromName(pixelPowerPin.c_str());

    // Pixel power control active level
    _pixelPowerActiveLevel = config.getBool("pixelPowerActiveLevel", true, pConfigPrefix);

    // Turn on power to LED pixels
    if (_pixelPowerPin >= 0)
    {
        pinMode(_pixelPowerPin, OUTPUT);
        digitalWrite(_pixelPowerPin, _pixelPowerActiveLevel);

#ifdef HOLD_PIXEL_POWER_PIN_DURING_SLEEP
        gpio_hold_en((gpio_num_t)_pixelPowerPin);
#endif
    }

    // Get LED pins
    bool rslt = _ledPixels.setup(config, pConfigPrefix);
    
    // Set animation count & timing
    _animationCount = _ledPixels.getNumPixels();
    _nextAnimStepAfterUs = 100000;

    // Log
#ifdef DEBUG_LED_GRID_SETUP
    LOG_I(MODULE_PREFIX, "setup %s numPixels %d pixelPowerPin %d activeLevel %s", 
                rslt ? "OK" : "FAILED", _ledPixels.getNumPixels(),
                _pixelPowerPin, _pixelPowerActiveLevel ? "HIGH" : "LOW");
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Service
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDGrid::service()
{
    // // Turn off any LEDs that need to be turned off
    // for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    // {
    //     if (_animationOffAfterUs[ledIdx] > 0)
    //     {
    //         if (Raft::isTimeout(micros(), _lastAnimTimeUs, _animationOffAfterUs[ledIdx]))
    //         {
    //             digitalWrite(_ledPins[ledIdx], !_ledActiveLevel);
    //             _animationOffAfterUs[ledIdx] = 0;
    //             // LOG_I(MODULE_PREFIX, "service setLedOff ledIdx %d", ledIdx);
    //         }
    //     }
    // }

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
            _animationColourIdx++;
            if (_animationColourIdx >= sizeof(_animationColours) / sizeof(_animationColours[0]))
                _animationColourIdx = 0;

            // TODO reinstate?
            // _nextAnimStepAfterUs = UINT32_MAX;
        }

        // Set next animation step time
        _lastAnimTimeUs = micros();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handle animation step
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDGrid::handleAnimationStep()
{
    _nextAnimStepAfterUs = 100000;
    _ledPixels.clear();
    _ledPixels.setPixelColor(_animationStepNum, _animationColours[_animationColourIdx]);
    _ledPixels.show();

//     // Set LEDs
//     for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
//     {
//         // Use animation step to determine LED state
//         int animStepIdx = (int)_animationStepNum - ledIdx;
//         if (animStepIdx >= 0)
//         {
//             if (animStepIdx < _animationStepLevels.size())
//             {
//                 // Set animation off after time
//                 int animOffTimeUs = _animationStepLevels[animStepIdx] * _ledIntensityFactors[ledIdx] * _displayBrightnessLevel;

//                 // Set LED state
//                 digitalWrite(_ledPins[ledIdx], animOffTimeUs != 0 ? _ledActiveLevel : !_ledActiveLevel);
//                 _animationOffAfterUs[ledIdx] = animOffTimeUs;

//                 // LOG_I(MODULE_PREFIX, "handleAnimationStep setLedOn ledIdx %d animStepIdx %d offTimeMs %d", 
//                 //         ledIdx, animStepIdx, _animationOffAfterMs[ledIdx]);
//             }
//         }
//     }

// #ifdef USE_FIXED_TIMER_FOR_LED_BRIGHTNESS
//     delay(1);
//     for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
//     {
//         digitalWrite(_ledPins[ledIdx], !_ledActiveLevel);
//     }

// #endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get time to next animation step
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t LEDGrid::getTimeToNextAnimStepUs()
{
    // // Check if any LEDs need to be turned off
    // uint32_t animOffTimeUs = UINT32_MAX;
    // for (int ledIdx = 0; ledIdx < _ledPins.size(); ledIdx++)
    // {
    //     if (_animationOffAfterUs[ledIdx] > 0)
    //     {
    //         if (_animationOffAfterUs[ledIdx] < animOffTimeUs)
    //             animOffTimeUs = _animationOffAfterUs[ledIdx];
    //     }
    // }

    // uint32_t timeUs = _nextAnimStepAfterUs < animOffTimeUs ? _nextAnimStepAfterUs : animOffTimeUs;
    // // LOG_I(MODULE_PREFIX, "getTimeToNextAnimStepMs nextAnimStepTimeMs %d animOffTimeMs %d timeMs %d", 
    // //         _nextAnimStepAfterUs, animOffTimeMs, timeMs);
    // return timeUs;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shutdown
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDGrid::shutdown()
{
    // Turn off power to LED pixels
    if (_pixelPowerPin >= 0)
    {
        digitalWrite(_pixelPowerPin, !_pixelPowerActiveLevel);
    }
}
