/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Phase-Locked Loop
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <stdio.h>
#include "PIDController.h"

// #define DEBUG_PLL

class PhaseLockedLoop
{
public:
    PhaseLockedLoop(double minFreqHz, double maxFreqHz, double centreFreqHz,
                double maxPIDOutput, 
                double kP, double kI, double kD) :
        _frequencyPID(kP, kI, kD, maxPIDOutput, -maxPIDOutput),
        _maxFreqHz(maxFreqHz), 
        _minFreqHz(minFreqHz), 
        _centreFreqHz(centreFreqHz),
        _beatFreqHz(centreFreqHz)
    {
    }
    ~PhaseLockedLoop()
    {
    }
    void processZeroCrossing(uint32_t sampleTimeMs)
    {
        if (_zeroCrossingFirstMs == 0)
        {
            _zeroCrossingFirstMs = sampleTimeMs;
            _lastZeroCrossingMs = sampleTimeMs;
            _beatFreqHz = _centreFreqHz;
            return;
        }

        // Check valid
        if (sampleTimeMs <= _lastZeroCrossingMs + 1)
            return;

        // Time between zero crossings
        uint32_t intervalMs = sampleTimeMs - _lastZeroCrossingMs;
        if (intervalMs == 0)
            return;

        // Save last zero crossing
        _lastZeroCrossingMs = sampleTimeMs;

        // Calculate frequency based on time between zero crossings
        double measuredFreqHz = 1000.0 / intervalMs;
        measuredFreqHz = std::clamp(measuredFreqHz, _minFreqHz, _maxFreqHz);

        // printf("PLL: %f %f %f %f\n", measuredFreqHz, _beatFreqHz, _frequencyPID.getMin(), _frequencyPID.getMax());

        // Update PID
#ifdef DEBUG_PLL
        double prevFreq = _beatFreqHz;
#endif
        _beatFreqHz -= _frequencyPID.process(_beatFreqHz, measuredFreqHz, intervalMs) * _scalingFactor;

#ifdef DEBUG_PLL
        printf(" prev beatFreq %f beatFreq %f\n", prevFreq, _beatFreqHz);
#endif
    }

    uint32_t timeToNextPeakMs(uint32_t curTimeMs)
    {
        // Interval in ms between zero crossings
        uint32_t intervalMs = 1000 / _beatFreqHz;
        
        // Time since last zero crossing
        uint32_t timeSinceLastZeroMs = curTimeMs - _lastZeroCrossingMs;

        // Time to next zero crossing
        uint32_t timeToNextZeroMs = intervalMs - timeSinceLastZeroMs;

        // Time to next peak is assumed to be quarter of the interval
        uint32_t timeToNextPeakMs = (timeToNextZeroMs + (3 * intervalMs / 4)) % intervalMs;
        return timeToNextPeakMs;
    }

    double getBeatFreqHz()
    {
        return _beatFreqHz;
    }

private:
    uint32_t _zeroCrossingFirstMs = 0;
    uint32_t _lastZeroCrossingMs = 0;
    PIDControl _frequencyPID;
    double _maxFreqHz = 3.5;
    double _minFreqHz = 0.5;
    double _centreFreqHz;
    double _beatFreqHz = 1.0;
    double _scalingFactor = 1.0;
};
