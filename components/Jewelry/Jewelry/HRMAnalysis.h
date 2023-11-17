/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Heart Rate Monitor (HRM) Analysis
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IIRFilter.h"
#include "ZeroCrossingDetector.h"
#include "PhaseLockedLoop.h"

class HRMAnalysis
{
public:
    HRMAnalysis() :
        // Coefficients generated using scipy butter filter b, a = signal.butter(2, 1.25, fs=50, btype='low')
        // Initial conditions zi0, zi1 = signal.lfilter_zi(b, a)
        _lowPassFilter(1, -1.7786317778245846, 0.8008026466657073, 0.005542717210280682, 0.011085434420561363, 0.005542717210280682, 0.9944572827897219, -0.7952599294554288),
        // Coefficients generated using scipy butter filter signal.butter(2, 0.75, fs=50, btype='high')
        // Initial conditions zi0, zi1 = signal.lfilter_zi(b, a)
        _highPassFilter(1, -1.86689228, 0.87521455, 0.93552671, -1.87105341, 0.93552671, -0.93552671, 0.93552671),
        // Phase locked loop
        // Parameters set highest and lowest expected heart rate in Hz and max PID output (+/-)
        _phaseLockedLoop(3.5, 0.5, 10)
    {
    }
    ~HRMAnalysis()
    {
    }
    void process(double sample, uint32_t sampleTimeMs)
    {
        // Filtering - first low pass filter then high pass filter
        double filteredSample = _lowPassFilter.process(sample);
        filteredSample = _highPassFilter.process(filteredSample);
        
        // Zero crossing detector
        bool isZeroCrossing = _zeroCrossingDetector.process(filteredSample, sampleTimeMs);

        // Phase locked loop
        if (isZeroCrossing)
            _phaseLockedLoop.processZeroCrossing(sampleTimeMs);
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

private:
    IIRFilter _lowPassFilter;
    IIRFilter _highPassFilter;
    ZeroCrossingDetector _zeroCrossingDetector;
    PhaseLockedLoop _phaseLockedLoop;
};
