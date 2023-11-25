///////////////////////////////////////////////////////////////////////////////
//
// Jewelry.cpp
//
// Rob Dobson 2023
//
///////////////////////////////////////////////////////////////////////////////

#include "Jewelry.h"
#include "RaftUtils.h"
#include "HeartEarring.h"
#include "GridEarring.h"

static const char *MODULE_PREFIX = "Jewelry";

// TODO TURN OFF
// #define USE_PIN_FOR_DEBUG 9

// The following stops the power control function keeping the board alive
#define ENABLE_POWER_CONTROL

// The following allows disabling of animations
#define ENABLE_ANIMATIONS

// The following disables sleeping
#define ENABLE_SLEEP_MODE

// Debug heart rate
// #define DEBUG_HEART_RATE

// Enable LED grid
#define ENABLE_LED_GRID

///////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
///////////////////////////////////////////////////////////////////////////////

Jewelry::Jewelry(const char *pModuleName, ConfigBase &defaultConfig, ConfigBase *pGlobalConfig, ConfigBase *pMutableConfig) :
    SysModBase(pModuleName, defaultConfig, pGlobalConfig, pMutableConfig)
{
}

Jewelry::~Jewelry()
{
}

///////////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////////

void Jewelry::setup()
{
    // Call base class
    SysModBase::setup();

#ifdef ENABLE_POWER_CONTROL
    // Setup power control
    _powerControl.setup(configGetConfig(), "PowerControl");
#endif

    // Check hardware type
    String hwTypeStr = configGetString("hardwareType", "");
    if (hwTypeStr == "heart")
    {
        // Setup heart earring
        _pJewelry = new HeartEarring();
        _pJewelry->setup(configGetConfig(), "HeartEarring");
    }
    else if (hwTypeStr == "grid")
    {
        // Setup grid earring
        _pJewelry = new GridEarring();
        _pJewelry->setup(configGetConfig(), "GridEarring");
    }

    // Debug
#ifdef USE_PIN_FOR_DEBUG
    pinMode(USE_PIN_FOR_DEBUG, OUTPUT);
    digitalWrite(USE_PIN_FOR_DEBUG, LOW);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Service
///////////////////////////////////////////////////////////////////////////////

void Jewelry::service()
{
    // Check jewelry valid
    if (_pJewelry)
    {
        // Service jewelry
        _pJewelry->service();

        // Update data collection if enabled
        String samplesJSON = _pJewelry->getLastSamplesJSON();
        if (samplesJSON.length() > 0)
        {
            sysModSendCmdJSON("HRMSamples", samplesJSON.c_str());
        }
    }

#ifdef ENABLE_POWER_CONTROL
    // Service power control
    _powerControl.service();

    // Check for shutdown
    if (_powerControl.isShutdownRequested())
    {
        // Debug
        LOG_I(MODULE_PREFIX, "service shutdown requested");
        delay(100);

        // Shutdown
        if (_pJewelry)
            _pJewelry->shutdown();

        // Shutdown
        _powerControl.shutdown();
    }
#endif
}
