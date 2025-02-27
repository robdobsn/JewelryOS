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
#include "LEDPatternRainbowSnake.h"
#include "LEDPatternScrollMsg.h"
#include "driver/gpio.h"

#define HOLD_PIXEL_PINS_DURING_SLEEP

#define DEBUG_LED_GRID_SETUP
// #define USE_FIXED_TIMER_FOR_LED_BRIGHTNESS

// #define ANIMATION_IN_THIS_CLASS

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

void LEDGrid::setup(const RaftJsonIF& config)
{
#ifdef FEATURE_OLD_LED_GRID

    // Get the pixel power control pin
    String pixelPowerPin = config.getString("pixelPowerPin", "");
    _pixelPowerPin = ConfigPinMap::getPinFromName(pixelPowerPin.c_str());

    // Pixel power control active level
    _pixelPowerActiveLevel = config.getBool("pixelPowerActiveLevel", true);

    // Turn on power to LED pixels
    if (_pixelPowerPin >= 0)
    {
        pinMode(_pixelPowerPin, OUTPUT);
        digitalWrite(_pixelPowerPin, _pixelPowerActiveLevel);
    }

    // Get LED pins
    bool rslt = _ledPixels.setup(config);

    // Set animation count & timing
    _animationCount = _ledPixels.getNumPixels();
    _nextAnimStepAfterUs = 100000;

    // Get LED grid size
    _gridWidth = config.getLong("gridWidth", 0);
    _gridHeight = config.getLong("gridHeight", 0);

    // Extract grid raster layout
    std::vector<String> gridElemStrs;
    config.getArrayElems("gridRaster", gridElemStrs);

    // Check grid raster size
    if (gridElemStrs.size() != _gridWidth * _gridHeight)
    {
        // No grid raster specified (or wrong size)
        LOG_E(MODULE_PREFIX, "setup gridRaster size %d does not match gridWidth %d gridHeight %d", 
                    gridElemStrs.size(), _gridWidth, _gridHeight);
    }
    else
    {
        // Convert grid raster
        _gridRaster.resize(gridElemStrs.size());
        for (int elemIdx = 0; elemIdx < gridElemStrs.size(); elemIdx++)
        {
            _gridRaster[elemIdx] = (uint8_t)gridElemStrs[elemIdx].toInt();
        }

        // Set mapping function for LED pixels
        _ledPixels.setPixelMappingFn(std::bind(&LEDGrid::mapPixelIdxToLEDIdx, this, std::placeholders::_1));
    }


    // Add patterns to LED pixels
    _ledPixels.addPattern("RainbowSnake", &LEDPatternRainbowSnake::build);
    _ledPixels.addPattern("ScrollMsg", &LEDPatternScrollMsg::build);
    _ledPixels.setPattern("ScrollMsg");

    // Log
#ifdef DEBUG_LED_GRID_SETUP
    LOG_I(MODULE_PREFIX, "setup %s numPixels %d pixelPowerPin %d activeLevel %s", 
                rslt ? "OK" : "FAILED", _ledPixels.getNumPixels(),
                _pixelPowerPin, _pixelPowerActiveLevel ? "HIGH" : "LOW");
#endif

#endif

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loop
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDGrid::loop()
{
#ifdef ANIMATION_IN_THIS_CLASS
    // Check if time for next step in sequence
    if (Raft::isTimeout(micros(), _lastAnimTimeUs, _nextAnimStepAfterUs))
    {
        // LOG_I(MODULE_PREFIX, "loop animStep %d animColourIdx %d", _animationStepNum, _animationColourIdx);
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
#else
    _ledPixels.loop();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handle animation step
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDGrid::handleAnimationStep()
{
    // _nextAnimStepAfterUs = 100000;
    // _ledPixels.clear();
    // _ledPixels.setRGB(_animationStepNum, _animationColours[_animationColourIdx]);
    // _ledPixels.show();

    // LOG_I(MODULE_PREFIX, "handleAnimationStep animStep %d animColourIdx %d colour %06x", 
    //             _animationStepNum, _animationColourIdx, _animationColours[_animationColourIdx]);

    // delayMicroseconds(1500);

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
// Wait for animation to complete
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDGrid::waitAnimComplete()
{
    _ledPixels.waitUntilShowComplete();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get time to next animation step
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t LEDGrid::getTimeToNextAnimStepUs()
{
    // uint32_t timeUs = _nextAnimStepAfterUs < animOffTimeUs ? _nextAnimStepAfterUs : animOffTimeUs;
    // LOG_I(MODULE_PREFIX, "getTimeToNextAnimStepMs nextAnimStepTimeMs %d animOffTimeMs %d timeMs %d", 
    //         _nextAnimStepAfterUs, animOffTimeMs, timeMs);
    // return timeUs;

    return _nextAnimStepAfterUs;
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pre and Post sleep
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LEDGrid::preSleep()
{
    // Hold power pin
#ifdef HOLD_PIXEL_PINS_DURING_SLEEP
    if (_pixelPowerPin >= 0)
        gpio_hold_en((gpio_num_t)_pixelPowerPin);
#endif
}

void LEDGrid::postSleep()
{
    // Release power pin
#ifdef HOLD_PIXEL_PINS_DURING_SLEEP
    if (_pixelPowerPin >= 0)
        gpio_hold_dis((gpio_num_t)_pixelPowerPin);
#endif
}
