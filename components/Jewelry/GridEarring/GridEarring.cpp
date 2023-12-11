/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Grid Earring
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GridEarring.h"
#include "esp_sleep.h"

static const char *MODULE_PREFIX = "GridEarring";

GridEarring::GridEarring()
{
}

GridEarring::~GridEarring()
{
}

void GridEarring::setup(const ConfigBase& config, const char* pConfigPrefix)
{
    // Config prefix base
    String configPrefix = String(pConfigPrefix) + "/";

    // Setup LED grid
    _ledGrid.setup(config, (configPrefix + "LEDGrid").c_str());

    // Set initialized
    _isInitialized = true;
}

void GridEarring::service()
{
    // Check initialized
    if (!_isInitialized)
        return;

    // Service
    _ledGrid.service();

    // Get time to next animation step
    // UINT32_MAX means that animation has finished
    uint32_t timeToNextAnimStepUs = _ledGrid.getTimeToNextAnimStepUs();
    if (timeToNextAnimStepUs == UINT32_MAX)
        return;

    // Cbeck animation complete
    _ledGrid.waitAnimComplete();

    // TODO PUT BACK

    // // Pre-sleep
    // _ledGrid.preSleep();

    // // Sleep for time to next animation step
    // esp_sleep_enable_timer_wakeup(timeToNextAnimStepUs);
    // esp_light_sleep_start();    

    // // Post-sleep
    // _ledGrid.postSleep();
}

void GridEarring::shutdown()
{
    // Check initialized
    if (!_isInitialized)
        return;

    // Shutdown LED grid
    _ledGrid.shutdown();
}
