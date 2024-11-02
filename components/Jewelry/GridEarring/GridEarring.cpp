/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Grid Earring
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GridEarring.h"
#include "esp_sleep.h"

// static const char *MODULE_PREFIX = "GridEarring";

#define SLEEP_BETWEEN_ANIMATION_STEPS

GridEarring::GridEarring()
{
}

GridEarring::~GridEarring()
{
}

void GridEarring::setup(const RaftJsonIF& config, DeviceManager& devMan)
{
    // Setup LED grid
    RaftJsonPrefixed configLEDGrid(config, "LEDGrid");
    _ledGrid.setup(configLEDGrid);

    // // Setup microphone
    // _microphone.setup(config, "Microphone".c_str());

    // Set initialized
    _isInitialized = true;
}

void GridEarring::loop()
{
    // Check initialized
    if (!_isInitialized)
        return;

    // Loop
    _ledGrid.loop();

    // // Microphone
    // _microphone.loop();

    // Get time to next animation step
    // UINT32_MAX means that animation has finished
    uint32_t timeToNextAnimStepUs = _ledGrid.getTimeToNextAnimStepUs();
    if (timeToNextAnimStepUs == UINT32_MAX)
        return;

    // Cbeck animation complete
    _ledGrid.waitAnimComplete();

    // TODO - remove
    delay(2);

#ifdef SLEEP_BETWEEN_ANIMATION_STEPS
    // Pre-sleep
    _ledGrid.preSleep();

    // Sleep for time to next animation step
    esp_sleep_enable_timer_wakeup(timeToNextAnimStepUs);
    esp_light_sleep_start();    

    // Post-sleep
    _ledGrid.postSleep();
#endif
}

void GridEarring::shutdown()
{
    // Check initialized
    if (!_isInitialized)
        return;

    // Shutdown LED grid
    _ledGrid.shutdown();

    // // Shutdown microphone
    // _microphone.teardown();
}
