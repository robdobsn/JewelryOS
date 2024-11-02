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
    virtual void setup(const RaftJsonIF& config, DeviceManager& devMan) override final;

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
#ifdef FEATURE_MAX30101_SENSOR
        return _max30101.wakeupOnGPIO();
#else
        return false;
#endif
    }

    // Debug check if samples available
    virtual bool debugAreSamplesAvailable() override final
    {
#ifdef FEATURE_MAX30101_SENSOR
        return _max30101.debugAreSamplesAvailable();
#else
        return false;
#endif
    }

    // Debug get last samples JSON
    virtual String debugGetLastSamplesJSON() override final
    {
#ifdef FEATURE_MAX30101_SENSOR
        return _max30101.debugGetLastSamplesJSON();
#else
        return "";
#endif
    }

    /// @brief Get named value
    /// @param valueName
    /// @param isValid
    /// @return double
    virtual double getNamedValue(const char* valueName, bool& isValid) override final;

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

#ifdef FEATURE_MAX30101_SENSOR
    // MAX30101
    MAX30101 _max30101;
#endif

#ifdef FEATURE_I2C_STANDALONE
    // I2C
    BusI2CESPIDF _i2cCentral;
    int _sdaPin = -1;
    int _sclPin = -1;
    int _freq = 100000;
#endif

    // Raft bus device decode state
    RaftBusDeviceDecodeState _decodeState;

    // LED heart display
    LEDHeart _ledHeart;

    // HRM analysis
    HRMAnalysis _hrmAnalysis;

    // Semaphore for access to heart rate anaylsis result
    SemaphoreHandle_t _heartRateValueMutex = nullptr;
    HRMAnalysis::HRMResult _hrmAnalysisResult;

    // Debug
    uint32_t _lastDebugTimeMs = 0;
    static constexpr const char *MODULE_PREFIX = "HeartEarring";

};
