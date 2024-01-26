#pragma once

#include <stdio.h>
#include <stdint.h>
#include "SimpleMovingAverage.h"

class RateEstimator
{
public:
    RateEstimator(int maxRateBPM, int minRateBPM)
    {
        max_interval_ms = 60000 / minRateBPM;
        min_interval_ms = 60000 / maxRateBPM;
    }
    ~RateEstimator()
    {
    }
    int process(int sample_interval_ms, int& confidence_percent)
    {
        if ((sample_interval_ms < min_interval_ms) || (sample_interval_ms > max_interval_ms))
        {
            confidence_percent = _confidence_percent;
            return _last_rate_bpm;
        }
        int sample_interval_avg_ms = _sample_interval_ms_avg.sample(sample_interval_ms);
        if (sample_interval_avg_ms == 0)
        {
            confidence_percent = _confidence_percent;
            return _last_rate_bpm;
        }
        int rate = 60000 / sample_interval_avg_ms;
        _last_rate_bpm = rate;

        // Confidence is based on the variance of the sample interval
        double std_dev = sqrt(_sample_interval_ms_avg.getVariance());
        _confidence_percent = 100 - (int)(std_dev * 100 / (max_interval_ms - min_interval_ms));

        printf("sample_interval_ms %d sample_interval_avg_ms %d rate %d std_dev %f confidence_percent %d\n",
                 sample_interval_ms, sample_interval_avg_ms, rate, std_dev, _confidence_percent);


        confidence_percent = _confidence_percent;
        return _last_rate_bpm;
    }

private:
    int _last_rate_bpm = 60;
    int _confidence_percent = 0;
    SimpleMovingAverage<50> _sample_interval_ms_avg;
    int min_interval_ms = 0;
    int max_interval_ms = INT32_MAX;
};