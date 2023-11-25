/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Grid Earring
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GridEarring.h"

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
}

void GridEarring::shutdown()
{
    // Check initialized
    if (!_isInitialized)
        return;

    // Shutdown LED grid
    _ledGrid.shutdown();
}
