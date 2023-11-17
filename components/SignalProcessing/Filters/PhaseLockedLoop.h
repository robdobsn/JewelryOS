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
    PhaseLockedLoop(float maxFreqHz, float minFreqHz, float maxPIDOutput) :
        _frequencyPID(5.5, 0.1, 0.2, maxPIDOutput, -maxPIDOutput)
    {
        _maxFreqHz = maxFreqHz;
        _minFreqHz = minFreqHz;
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
            return;
        }

        // Check valid
        if (sampleTimeMs <= _lastZeroCrossingMs)
            return;

        // Time between zero crossings
        uint32_t intervalMs = sampleTimeMs - _lastZeroCrossingMs;

        // Save last zero crossing
        _lastZeroCrossingMs = sampleTimeMs;

        // Calculate frequency based on time between zero crossings
        float measuredFreqHz = 1000.0 / intervalMs;

        // printf("PLL: %f %f %f %f\n", measuredFreqHz, _beatFreqHz, _frequencyPID.getMin(), _frequencyPID.getMax());

        // Check in valid range
        if ((measuredFreqHz < _minFreqHz) || (measuredFreqHz > _maxFreqHz))
            return;

        // Update PID
        _beatFreqHz -= _frequencyPID.process(_beatFreqHz, measuredFreqHz, intervalMs) * 0.1;

#ifdef DEBUG_PLL
        printf(" beatFreq %f\n", _beatFreqHz);
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

    float getBeatFreqHz()
    {
        return _beatFreqHz;
    }

private:
    float _beatFreqHz = 1.0;
    uint32_t _zeroCrossingFirstMs = 0;
    uint32_t _lastZeroCrossingMs = 0;
    PIDControl _frequencyPID;
    float _maxFreqHz = 3.5;
    float _minFreqHz = 0.5;
};
