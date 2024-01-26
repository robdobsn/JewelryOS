#pragma once
#include <stdio.h>

class ZeroCrossingDetector_Eval
{
public:
    ZeroCrossingDetector_Eval()
    {
    }
    ~ZeroCrossingDetector_Eval()
    {
    }
    bool process(int sample, int sample_time)
    {
        // Check for first sample
        if (_lastSampleTimeMs == 0)
        {
            _lastSample = sample;
            _lastSampleTimeMs = sample_time;
            return false;
        }
        bool result = false;
        if (_lastSampleWasPositive && sample < 0)
        {
            result = true;
        }
        _lastSampleWasPositive = sample >= 0;
        _lastSample = sample;
        _lastSampleTimeMs = sample_time;
        return result;
    }

private:
    int _lastSample = 0;
    int _lastSampleTimeMs = 0;
    bool _lastSampleWasPositive = false;
};