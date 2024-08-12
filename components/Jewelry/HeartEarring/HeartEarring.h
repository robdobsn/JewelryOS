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
    virtual void setup(const RaftJsonIF& config) override final;

    // Loop
    virtual void loop() override final;

    // Get sleep duration in us
    virtual uint32_t getSleepDurationUs() override final
    {
        return _nextSleepDurationUs;
    }

    // Shutdown
    virtual void shutdown() override final;

    // Check if we should wakeup on GPIO
    bool wakeupOnGPIO()
    {
        return _max30101.wakeupOnGPIO();
    }

    // Debug check if samples available
    virtual bool debugAreSamplesAvailable() override final
    {
        return _max30101.debugAreSamplesAvailable();
    }

    // Debug get last samples JSON
    virtual String debugGetLastSamplesJSON() override final
    {
        return _max30101.debugGetLastSamplesJSON();
    }

private:
    // Time between heart pulses
    uint32_t _timeToNextPulseAnimStartUs = 1000000;

    // Animation timing
    uint64_t _timeOfLastStepUs = 0;

    // Sleep duration in us - this is continually updated (in the loop function) to account 
    // for the time to the next pulse animation step
    uint32_t _nextSleepDurationUs = 0;

    // Animation mode
    bool _isPulseStart = true;

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

    // Debug
    uint32_t _lastDebugTimeMs = 0;
};
