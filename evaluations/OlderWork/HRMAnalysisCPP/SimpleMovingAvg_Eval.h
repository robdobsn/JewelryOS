
/*
 * Moving Average
 *
 * Calculate the average of a value over a moving window of N samples
 * 
 * Template N is the window size
 * Template input_t is the type of the input value
 * Template sum_t is the type of the sum
 * 
 */

#pragma once
#include <stdint.h>

template <uint16_t N, class input_t = uint32_t, class sum_t = uint64_t>
class SimpleMovingAverage_Eval
{
public:
    SimpleMovingAverage_Eval()
    {
        clear();
    }
    input_t sample(input_t input) 
    {
        // Update sum removing oldest and adding newest
        sum -= previousInputs[index];
        sum += input;
        previousInputs[index] = input;
        // Bump circular index
        if (++index == N)
            index = 0;
        // Add to number of entries if below N
        if (numEntries < N)
            numEntries++;
        // Calculate result
        return sum / numEntries;
    }

    input_t getAverage() 
    {
        return sum / numEntries;
    }

    void clear()
    {
        index = 0;
        numEntries = 0;
        sum = 0;
        for (uint8_t i = 0; i < N; i++)
            previousInputs[i] = 0;
    }

    double getVariance()
    {
        if (numEntries == 0)
            return 0;
        double mean = getAverage();
        double variance = 0;
        for (uint8_t i = 0; i < numEntries; i++)
        {
            double diff = previousInputs[i] - mean;
            variance += diff * diff;
        }
        variance /= numEntries;
        return variance;
    }

private:
    uint16_t index = 0;
    uint16_t numEntries = 0;
    input_t previousInputs[N] = {};
    sum_t sum = 0;
};
