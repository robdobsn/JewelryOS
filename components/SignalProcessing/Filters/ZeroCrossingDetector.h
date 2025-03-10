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
    bool process(int sample, bool bothEdges)
    {
        // Check for first sample
        if (_isFirstSample)
        {
            _lastSample = sample;
            _isFirstSample = false;
            return false;
        }

        // Check for zero crossing
        bool result = false;
        if (_lastSampleWasPositive && (sample < 0))
        {
            result = true;
        }
        else if (bothEdges && (!_lastSampleWasPositive && (sample >= 0)))
        {
            result = true;
        }

        // Store last sample
        _lastSampleWasPositive = sample >= 0;
        _lastSample = sample;
        return result;
    }

private:
    int _lastSample = 0;
    bool _lastSampleWasPositive = false;
    bool _isFirstSample = true;
    uint32_t _lastZeroCrossingTimeMs = 0;
};
