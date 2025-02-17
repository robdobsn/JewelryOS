/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LED Heart
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "JewelryBase.h"
#include "LEDHeart.h"
#include "HRMAnalysis.h"
#include "RaftBusDevicesIF.h"
#include "RaftThreading.h"

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
        return false;
    }

    // Debug check if samples available
    virtual bool debugAreSamplesAvailable() override final
    {
        bool isAvailable = false;
        if (RaftMutex_lock(_lastSamplesJSONMutex, 2))
        {
            // Flag
            isAvailable = _lastSamplesJSON.length() > 0;

            // Give back the semaphore
            RaftMutex_unlock(_lastSamplesJSONMutex);
        }
        return isAvailable;       
    }

    // Debug get last samples JSON
    virtual String debugGetLastSamplesJSON() override final
    {
        String samplesJSON;
        if (RaftMutex_lock(_lastSamplesJSONMutex, 2))
        {
            // Flag
            samplesJSON = _lastSamplesJSON;
            _lastSamplesJSON = "";

            // Give back the semaphore
            RaftMutex_unlock(_lastSamplesJSONMutex);
        }    
        return samplesJSON;
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

    // Raft bus device decode state
    RaftBusDeviceDecodeState _decodeState;

    // LED heart display
    LEDHeart _ledHeart;

    // HRM analysis
    HRMAnalysis _hrmAnalysis;

    // Semaphore for access to heart rate anaylsis result
    RaftMutex _heartRateValueMutex;
    HRMAnalysis::HRMResult _hrmAnalysisResult;

    // HRM samples
    bool _collectHRM = false;
    String _lastSamplesJSON;
    RaftMutex _lastSamplesJSONMutex;

    // Debug
    uint32_t _lastDebugTimeMs = 0;
    static constexpr const char *MODULE_PREFIX = "HeartEarring";
};
