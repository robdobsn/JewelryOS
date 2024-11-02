/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Base class for specific jewelry types
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RaftArduino.h"
#include "RaftJsonIF.h"

// Disable this when not debugging
// #define DEBUG_USE_GPIO_PIN_FOR_TIMING 9

class DeviceManager;

class JewelryBase
{
public:

    // Constructor / Destructor
    JewelryBase()
    {
    }
    virtual ~JewelryBase()
    {
    }

    // Setup
    virtual void setup(const RaftJsonIF& config, DeviceManager& devMan) = 0;

    // Loop
    virtual void loop() = 0;

    // Get sleep duration in us
    virtual uint32_t getSleepDurationUs()
    {
        return 0;
    }

    // Shutdown
    virtual void shutdown() = 0;

    // Check if we should wakeup on GPIO
    bool wakeupOnGPIO()
    {
        return false;
    }

    // Debug check if samples available
    virtual bool debugAreSamplesAvailable()
    {
        return false;
    }

    // Debug get last samples JSON
    virtual String debugGetLastSamplesJSON()
    {
        return "";
    }

    /// @brief Get named value
    /// @param valueName
    /// @param isValid
    /// @return double
    virtual double getNamedValue(const char* valueName, bool& isValid)
    {
        isValid = false;
        return 0;
    }

    // Get isInitialized
    bool isInitialized()
    {
        return _isInitialized;
    }

protected:
    // Is initialized
    bool _isInitialized = false;    
};
