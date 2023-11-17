/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Zero crossing detector
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>

class ZeroCrossingDetector
{
public:
    ZeroCrossingDetector()
    {
    }
    ~ZeroCrossingDetector()
    {
    }
    bool process(int sample, int sampleTimeMs)
    {
        // Check for first sample
        if (_lastSampleTimeMs == 0)
        {
            _lastSample = sample;
            _lastSampleTimeMs = sampleTimeMs;
            return false;
        }

        // Check for zero crossing
        bool result = false;
        if (_lastSampleWasPositive && (sample < 0))
        {
            result = true;
        }

        // Store last sample
        _lastSampleWasPositive = sample >= 0;
        _lastSample = sample;
        _lastSampleTimeMs = sampleTimeMs;
        return result;
    }

private:
    int _lastSample = 0;
    int _lastSampleTimeMs = 0;
    bool _lastSampleWasPositive = false;
};
