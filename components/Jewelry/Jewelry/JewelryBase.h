/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Base class for specific jewelry types
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RaftArduino.h"
#include "ConfigBase.h"

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
    virtual void setup(const ConfigBase& config, const char* pConfigPrefix) = 0;

    // Service
    virtual void service() = 0;

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
