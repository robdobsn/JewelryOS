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
    virtual void setup(const RaftJsonIF& config) = 0;

    // Loop
    virtual void loop() = 0;

    // Shutdown
    virtual void shutdown() = 0;

    // Get last samples JSON
    virtual String getLastSamplesJSON()
    {
        return "";
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
