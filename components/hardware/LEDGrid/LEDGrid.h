/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Grid Display
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <RaftJsonIF.h>
#include <LEDPixels.h>

class LEDGrid
{
public:
    // Constructor / Destructor
    LEDGrid();
    ~LEDGrid();

    // Setup
    void setup(const RaftJsonIF& config);

    // Loop
    void loop();

    // Shutdown
    void shutdown();

    // Pre and Post sleep
    void preSleep();
    void postSleep();

    // Wait for animation to complete
    void waitAnimComplete();

    // Get time to next animation step
    uint32_t getTimeToNextAnimStepUs();

    // Layout
    uint32_t getGridWidth() const
    {
        return _gridWidth;
    }
    uint32_t getGridHeight() const
    {
        return _gridHeight;
    }
    uint32_t getGridSize() const
    {
        return _gridWidth * _gridHeight;
    }
    uint32_t getGridIdx(uint32_t x, uint32_t y) const
    {
        return y * _gridWidth + x;
    }

private:

    // Brightness level
    uint32_t _displayBrightnessPercent = 100;

    // LED pixels
    LEDPixels _ledPixels;

    // Pixel power control pin
    int _pixelPowerPin = -1;
    bool _pixelPowerActiveLevel = false;

    // Timer for animation
    uint64_t _lastAnimTimeUs = 0;

    // Next animation step time
    uint32_t _nextAnimStepAfterUs = 50000;

    // Animation step number & count
    uint32_t _animationStepNum = 0;
    uint32_t _animationCount = 0;

    // Animation off time for LEDs
    uint32_t _animationOffAfterUs = 0;

    // Colour for animation
    uint32_t _animationColourIdx = 0;
    static constexpr uint32_t _animationColours[] = { 0x0000ff, 0x00ff00, 0xff0000, 0x00ffff, 0xff00ff, 0xffff00, 0xffffff };

    // Grid size and raster layout
    uint32_t _gridWidth = 0;
    uint32_t _gridHeight = 0;
    std::vector<uint8_t> _gridRaster;

    // Helpers
    void handleAnimationStep();
    uint32_t mapPixelIdxToLEDIdx(uint32_t pixelIdx)
    {
        if (pixelIdx >= _gridRaster.size())
            return 0;
        return _gridRaster[pixelIdx];
    }
};
