/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Heart
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "JewelryBase.h"
#include "MAX30101.h"
#include "BusI2CESPIDF.h"
#include "LEDHeart.h"
#include "HRMAnalysis.h"

class HeartEarring : public JewelryBase
{
public:

    // Constructor / Destructor
    HeartEarring();
    virtual ~HeartEarring();

    // Setup
    virtual void setup(const ConfigBase& config, const char* pConfigPrefix) override final;

    // Service
    virtual void service() override final;

    // Shutdown
    virtual void shutdown() override final;

    // Get last samples JSON
    virtual String getLastSamplesJSON() override final
    {
        return _max30101.getLastSamplesJSON();
    }

private:

    // Debug
    uint32_t _lastDebugTimeMs = 0;
    
    // Time between heart pulses
    uint32_t _timeBetweenHeartPulsesMs = 1000;
    uint32_t _lastHeartPulseTimeMs = 0;

    // MAX30101
    MAX30101 _max30101;

    // I2C
    BusI2CESPIDF _i2cCentral;
    int _sdaPin = -1;
    int _sclPin = -1;
    int _freq = 100000;

    // LED heart display
    LEDHeart _ledHeart;

    // HRM analysis
    HRMAnalysis _hrmAnalysis;
};
