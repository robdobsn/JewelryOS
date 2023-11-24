/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Heart
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class LEDHeart
{
public:

    // Constructor / Destructor
    LEDHeart();
    ~LEDHeart();

    // Setup
    void setup();

    // Service
    void service();

    // Shutdown
    void shutdown();
    
    // Get time to next animation step
    uint32_t getTimeToNextAnimStepUs();

    // Start pulse animation
    void startPulseAnimation();

private:
};
