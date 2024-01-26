#pragma once

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "PIDControl_Eval.h"

#define MATCH_FREQUENCY

class PhaseLockedLoop_Eval
{
public:
    PhaseLockedLoop_Eval(float maxFreqHz, float minFreqHz, float maxPIDOutput) :
#ifdef MATCH_FREQUENCY
        _frequencyPID(0.5, 0.03, 0.2, maxPIDOutput, -maxPIDOutput)
#else
        _phasePID(1, 0, 0, maxPIDOutput, -maxPIDOutput)
#endif
    {
        _maxFreqHz = maxFreqHz;
        _minFreqHz = minFreqHz;
    }
    ~PhaseLockedLoop_Eval()
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

        // // Expected time to next peak
        // int expectedNextPeakMs = timeToNextPeakMs(sampleTimeMs);

        // // Calculate error
        // int errorMs = (expectedNextPeakMs > sampleTimeMs) ? expectedNextPeakMs - sampleTimeMs : sampleTimeMs - expectedNextPeakMs;

        // Update phase error
        // _phase_error_ms.sample(errorMs);

        // Update beat frequency
        // _beatFreqHz = 1000.0 / sampleTimeMs;

        // Check valid
        if (sampleTimeMs <= _lastZeroCrossingMs)
            return;

#ifdef MATCH_FREQUENCY

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

#else
        // Calculate number of cycles since first zero crossing
        int numCycles = round(_beatFreqHz * ((sampleTimeMs - _zeroCrossingFirstMs) / 1000.0));

        // Expected zero crossing ms
        float expectedZeroCrossingSecs = (numCycles * _beatFreqHz) + (_zeroCrossingFirstMs / 1000.0);

        _beatFreqHz -= _phasePID.process(expectedZeroCrossingSecs, sampleTimeMs/1000.0, 1) * 0.1;

#endif
        printf(" beatFreq %f\n", _beatFreqHz);
    }

    uint32_t timeToNextPeakMs(uint32_t curTimeMs)
    {
        // // Interval in ms between zero crossings
        // uint32_t intervalMs = 1000 / _beatFreqHz;
        // // Time in ms since last zero crossing
        // uint32_t timeSinceLastZeroCrossingMs = curTimeMs - _zeroCrossingFirstMs;
        // // Time to peak is assumed to be half the interval
        // uint32_t timeSinceLastPeakMs = (timeSinceLastZeroCrossingMs + (intervalMs / 2)) % intervalMs;
        // return intervalMs - timeSinceLastPeakMs;

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
    // SimpleMovingAverage<50, int32_t, int64_t> _phase_error_ms;
#ifdef MATCH_FREQUENCY
    PIDControl_Eval _frequencyPID;
#else
    PIDControl_Eval _phasePID;
#endif
    float _maxFreqHz = 3.5;
    float _minFreqHz = 0.5;
};