/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Heart Rate Monitor (HRM) Analysis
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IIRFilter4thOrder.h"
#include "ZeroCrossingDetector.h"
#include "PhaseLockedLoop.h"
#include <vector>

class HRMAnalysis
{
public:
    HRMAnalysis(double freqBandLowerHz = 0.75, double freqBandUpperHz = 3.0, double freqCentreHz = 1.0) :
        // Bandpass filter
        _butterBandpassFilter(_butterCoeff4A, _butterCoeff4B, _butterZi),

        // Phase locked loop
        // Parameters set highest and lowest expected heart rate in Hz and max PID output (+/-)
        _phaseLockedLoop(freqBandLowerHz, freqBandUpperHz, freqCentreHz, maxPIDOutput, kP_PID, kI_PID, kD_PID)
    {
    }
    ~HRMAnalysis()
    {
    }

    struct HRMResult
    {
        double heartRateHz = 0;
        uint32_t timeOfNextPeakMs = 0;
        uint32_t heartRatePulseIntervalMs = 0;
    };

    HRMResult process(double sample, uint32_t sampleTimeMs)
    {
        // Filtering
        double filteredSample = _butterBandpassFilter.process(sample);
        _debugFilteredSample = filteredSample;
        
        // Zero crossing detector
        bool isZeroCrossing = _zeroCrossingDetector.process(filteredSample, false);
        _debugIsZeroCrossing = isZeroCrossing;

        // Phase locked loop
        if (isZeroCrossing)
            _phaseLockedLoop.processZeroCrossing(sampleTimeMs);

        // Return beat frequency
        return HRMResult{getHeartRateHz(), 
                    getTimeOfNextPeakMs(sampleTimeMs), 
                    getHeartRatePulseIntervalMs()};
    }

    // Get heart rate
    double getHeartRateHz()
    {
        return _phaseLockedLoop.getBeatFreqHz();
    }

    // Get time to next peak
    uint32_t getTimeToNextPeakMs(uint32_t curTimeMs)
    {
        return _phaseLockedLoop.timeToNextPeakMs(curTimeMs);
    }

    // Get time of next peak
    uint32_t getTimeOfNextPeakMs(uint32_t curTimeMs)
    {
        uint32_t getTimeToNextPeakMs = _phaseLockedLoop.timeToNextPeakMs(curTimeMs);
        return curTimeMs + getTimeToNextPeakMs;
    }

    // Get heart rate pulse interval ms
    uint32_t getHeartRatePulseIntervalMs()
    {
        return 1000 / _phaseLockedLoop.getBeatFreqHz();
    }

    // Debug values
    double _debugFilteredSample = 0;
    bool _debugIsZeroCrossing = false;

private:
    // 25Hz sampling 8th order
    // static constexpr double _butterCoeff8A[] = {1.0, -6.05960751, 16.45391545, -26.18801509, 26.74607739, -17.95499326, 7.73746173, -1.9574456, 0.22281157};
    // static constexpr double _butterCoeff8B[] = {0.00336282, 0.0, -0.01345126, 0.0, 0.02017689, 0.0, -0.01345126, 0.0, 0.00336282};
    // // static constexpr double _butterZi[] = {-0.00336282, -0.00336282, 0.01008845, 0.01008845, -0.01008845, -0.01008845, 0.00336282, 0.00336282};
    // static constexpr double _butter8Zi[] = {0,0,0,0,0,0,0,0};

    // 25Hz sampling 4th order
    static constexpr double _butterCoeff4A[] = {1.0, -2.99198635, 3.52764744, -1.97218019, 0.45044543};
    static constexpr double _butterCoeff4B[] = {0.05644846, 0.0, -0.11289692, 0.0, 0.05644846};
    static constexpr double _butterZi[] = {-0.05644846, -0.05644846, 0.05644846, 0.05644846};
    static constexpr double maxPIDOutput = 10.0;
    static constexpr double kP_PID = 0.00005;
    static constexpr double kI_PID = 0.000005;
    static constexpr double kD_PID = 0.0005;

    IIRFilter4thOrder _butterBandpassFilter;
    ZeroCrossingDetector _zeroCrossingDetector;
    PhaseLockedLoop _phaseLockedLoop;
};
