#include <string>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <fstream>

// #include "SO_BPF.h"
#include "IIR1/Butterworth.h"
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#define MATCH_FREQUENCY

std::string findLatestLogFile(const std::string &logFilePath)
{
    std::string latest_file;
    std::filesystem::file_time_type latest_time = std::filesystem::file_time_type::min();

    for (const auto &entry : std::filesystem::directory_iterator(logFilePath))
    {
        if (entry.path().extension() == ".log")
        {
            auto ftime = std::filesystem::last_write_time(entry);
            if (ftime > latest_time)
            {
                latest_time = ftime;
                latest_file = entry.path().string();
            }
        }
    }

    if (latest_file.empty())
    {
        throw std::runtime_error("No log files found in " + logFilePath);
    }

    return latest_file;
}

std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::pair<std::vector<int>, std::vector<int>> readHRMData(std::string logfile)
{
    std::vector<int> timestamps;
    std::vector<int> red_led_adc_values;
    int first_sample_line_time_ms = 0;

    std::ifstream file(logfile);
    std::string line;

    while (std::getline(file, line))
    {
        if (line.find("samples") == std::string::npos)
        {
            continue;
        }

        std::vector<std::string> words = split(line, ' ');

        int line_time_ms = std::stoi(words[1].substr(1, words[1].size() - 2));

        auto it = std::find(words.begin(), words.end(), "sampleRate");
        float sample_rate = std::stof(*(it + 1));

        it = std::find(words.begin(), words.end(), "numSamples");
        int num_samples = std::stoi(*(it + 1));

        // Check for 0 samples
        if (num_samples == 0)
        {
            continue;
        }

        it = std::find(words.begin(), words.end(), "samples");

        if (first_sample_line_time_ms == 0)
        {
            first_sample_line_time_ms = line_time_ms;
        }

        int line_start_ms = line_time_ms - first_sample_line_time_ms;

        for (int i = 0; i < num_samples; ++i)
        {
            int sample_value = std::stoi(*(it + 1 + i));
            timestamps.push_back(line_start_ms + (i / sample_rate) * 1000);
            red_led_adc_values.push_back(sample_value);
        }
    }

    return {timestamps, red_led_adc_values};
}

// class BiquadBPFFilter
// {
// public:
//     BiquadBPFFilter()
//     {
//         // Our data is actually 50Hz and we want centre freequency of 1.5Hz
//         // but the args to calculate_coeffs are integers so the
//         // values for fc and fs are scaled up by a factor of 1000
//         // _bpf.calculate_coeffs(.5, 500, 50000);
//         SO_BPF::tp_coeffs coeffs = {
//             0.21279373587133188,
//             0,
//             -0.21279373587133188,
//             -1.524949463540011,
//             0.5744125282573361
//         };
//         _bpf.set_coefficients(coeffs);

//     }
//     ~BiquadBPFFilter()
//     {
//     }
//     int process(int sample)
//     {
//         return _bpf.process(sample);
//     }

// private:
//     SO_BPF _bpf;
// };

// BiquadBPFFilter bpf;
// SO_BPF bpf;

class IIRFilter
{
public:
    IIRFilter(double a0, double a1, double a2, double b0, double b1, double b2, double zi0, double zi1)
    {
        a[0] = a0;
        a[1] = a1;
        a[2] = a2;
        b[0] = b0;
        b[1] = b1;
        b[2] = b2;
        zi[0] = zi0;
        zi[1] = zi1;
        // _bpf.setup
        // // Our data is actually 50Hz and we want centre freequency of 1.5Hz
        // // but the args to calculate_coeffs are integers so the
        // // values for fc and fs are scaled up by a factor of 1000
        // // _bpf.calculate_coeffs(.5, 500, 50000);
        // // _bpf.setup(500, 10, 10);
        // _bpf.setup(50, 2, 2);
        // // _bpf.setup(50, 24);

        // // printf("BPF: %f %f %f %f %f %f\n", _bpf.getA0(), _bpf.getA1(), _bpf.getA2(), _bpf.getB1(), _bpf.getB2(), _bpf.getC0());
        // bhigh: [ 0.93552671 -1.87105341  0.93552671] ahigh: [ 1.         -1.86689228  0.87521455] zihigh: [-0.93552671  0.93552671]


    }
    ~IIRFilter()
    {
    }
    double process(double x1) {
        if (_isFirst)
        {
            v1m1 = zi[0] * x1;
            v2m1 = zi[1] * x1;
            _isFirst = false;
        }
        double y1 = 0;
        y1 = (b[0] * x1 + v1m1) / a[0];
        v1m = (b[1] * x1 + v2m1) - a[1] * y1;
        v2m = b[2] * x1 - a[2] * y1;
        v1m1 = v1m;
        v2m1 = v2m;
        if (count < 100)
        {
            printf("i: %d x1: %f y1: %f v1m1: %f v2m1: %f v1m: %f v2m: %f b: %f %f %f a: %f %f %f\n",
                    count, x1, y1, v1m1, v2m1, v1m, v2m, b[0], b[1], b[2], a[0], a[1], a[2]);
            count++;
        }
        return y1;
    }       

private:
    double a[3] = {1, -1.7786317778245846, 0.8008026466657073};
    double b[3] = {0.005542717210280682, 0.011085434420561363, 0.005542717210280682};
    double zi[2] = {0.9944572827897219, -0.7952599294554288};
    double v1m1 = 0, v2m1 = 0, v1m, v2m;
    bool _isFirst = true;
    uint32_t count = 0;

    // Iir::Butterworth::BandPass<2> _bpf;
};

IIRFilter lowPassFilter(1, -1.7786317778245846, 0.8008026466657073, 0.005542717210280682, 0.011085434420561363, 0.005542717210280682, 0.9944572827897219, -0.7952599294554288);
IIRFilter highPassFilter(1, -1.86689228, 0.87521455, 0.93552671, -1.87105341, 0.93552671, -0.93552671, 0.93552671);

class ZeroCrossingDetector
{
public:
    ZeroCrossingDetector()
    {
    }
    ~ZeroCrossingDetector()
    {
    }
    bool process(int sample, int sample_time)
    {
        // Check for first sample
        if (_last_sample_time == 0)
        {
            _last_sample = sample;
            _last_sample_time = sample_time;
            return false;
        }
        bool result = false;
        if (_last_sample_was_positive && sample < 0)
        {
            result = true;
        }
        _last_sample_was_positive = sample >= 0;
        _last_sample = sample;
        _last_sample_time = sample_time;
        return result;
    }

private:
    int _last_sample = 0;
    int _last_sample_time = 0;
    bool _last_sample_was_positive = false;
};

ZeroCrossingDetector zero_crossing_detector;

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

template <uint16_t N, class input_t = uint32_t, class sum_t = uint64_t>
class SimpleMovingAverage
{
public:
    SimpleMovingAverage()
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

private:
    uint16_t index = 0;
    uint16_t numEntries = 0;
    input_t previousInputs[N] = {};
    sum_t sum = 0;
};


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
        _confidence_percent = 100;
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

RateEstimator rate_estimator(200, 40);

class PIDControl
{
public:
    PIDControl(float kp, float ki, float kd, float max, float min)
    {
        _kp = kp;
        _ki = ki;
        _kd = kd;
        _max = max;
        _min = min;
    }
    ~PIDControl()
    {
    }
    float process(float setPoint, float processVariable, uint32_t timeDeltaMs)
    {
        // Calculate error
        float error = setPoint - processVariable;

        // Time delta in seconds
        float timeDeltaSecs = timeDeltaMs / 1000.0;
        if (timeDeltaSecs <= 0)
            return 0;

        // Proportional term
        float pOut = _kp * error;

        // Integral term
        if (timeDeltaSecs > 0)
            _integral += error * timeDeltaSecs;
        float iOut = _ki * _integral;

        // Derivative term
        float derivative = 0;
        if (timeDeltaSecs > 0)
            derivative = (error - _lastError) / timeDeltaSecs;
        float dOut = _kd * derivative;

        // Calculate total output
        float output = pOut + iOut + dOut;

        // Restrict to max/min
        if (output > _max)
            output = _max;
        else if (output < _min)
            output = _min;

        // Save error to previous error
        _lastError = error;

        printf("PID: %f %f %f %f %f %f %f %f %f", setPoint, processVariable, timeDeltaSecs, error, pOut, iOut, dOut, output, _integral);

        return output;
    }
private:
    float _kp;
    float _ki;
    float _kd;
    float _max;
    float _min;
    float _lastError = 0;
    float _integral = 0;
};

class PhaseLockedLoop
{
public:
    PhaseLockedLoop(float maxFreqHz, float minFreqHz, float maxPIDOutput) :
#ifdef MATCH_FREQUENCY
        _frequencyPID(5.5, 0.1, 0.2, maxPIDOutput, -maxPIDOutput)
#else
        _phasePID(1, 0, 0, maxPIDOutput, -maxPIDOutput)
#endif
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
    PIDControl _frequencyPID;
#else
    PIDControl _phasePID;
#endif
    float _maxFreqHz = 3.5;
    float _minFreqHz = 0.5;
};

PhaseLockedLoop phaseLockedLoop(3.5, 0.5, 10);

// // IIR bandpass filter
// std::vector<int> filterHRMData(std::vector<int> timestamps, std::vector<int> red_led_adc_values) {
//     std::vector<int> filtered_red_led_adc_values;
//     // bpf.calculate_coeffs(0.5, 500, 50000);
//     for (int i = 0; i < red_led_adc_values.size(); ++i) {
//         filtered_red_led_adc_values.push_back(bpf.process(red_led_adc_values[i]));
//     }
//     return filtered_red_led_adc_values;
// }

// #define OVERRIDE_DATA_WITH_TEST_WAVEFORMS

int main()
{
    std::string logFilePath = "../../logs";
    std::string latestLogFile = findLatestLogFile(logFilePath);
    // Read file data
    auto [timestamps, red_led_adc_values] = readHRMData(latestLogFile);


#ifdef OVERRIDE_DATA_WITH_TEST_WAVEFORMS

    red_led_adc_values.clear();
    // Create a 1Hz sine wave at 50 samples per second
    float startFreqHz = 3;
    float endFreqHz = 0.8;
    const float sampleRateHz = 50.0;
    float duration = (timestamps[timestamps.size()-1]-timestamps[0])/1000.0;
    // float aFactor = 2 * M_PI * (maxFreqHz-minFreqHz) / sampleRateHz;
    // float bFactor = 2 * M_PI * minFreqHz / sampleRateHz;
    for (int i = 0; i < timestamps.size(); ++i)
    {
        float t = (timestamps[i]/1000.0);
        float outVal = cos(2 * M_PI * (startFreqHz * t + (endFreqHz - startFreqHz) * t * t / (2 * duration)));
        red_led_adc_values.push_back(1000 * outVal + 35000);
        // if ((freqHz < 0.8) && (freqIncreaseRateHz < 0))
        //     freqIncreaseRateHz = -freqIncreaseRateHz;
        // else if ((freqHz > 2) && (freqIncreaseRateHz > 0))
        //     freqIncreaseRateHz = -freqIncreaseRateHz;
        // if (i % 100 == 0)
        //     printf("%d %d Freq %fHz\n", timestamps[i], red_led_adc_values[i], freqHz);
        if (i < 100)
            printf("%d out %d t %f\n", timestamps[i], red_led_adc_values[i], t);
    }
#endif


    // Process data sequentially
    std::vector<int> filtered_red_led_adc_values;
    std::vector<int> zero_crossing_intervals_ms;
    std::vector<int> rate_estimations;
    std::vector<int> confidence_percents;
    std::vector<uint32_t> timeToNextPeaksMs;
    std::vector<float> beatFreqHzs;
    int prev_zero_crossing_timestamp = 0;
    int rate_estimation = 0;
    for (int i = 0; i < red_led_adc_values.size(); ++i)
    {
        // IIR bandpass filter
        int filtered_sample = lowPassFilter.process(red_led_adc_values[i]);
        filtered_sample = highPassFilter.process(filtered_sample);
        filtered_red_led_adc_values.push_back(filtered_sample);

        // Zero crossing detector
        bool zero_crossing = zero_crossing_detector.process(filtered_sample, timestamps[i]);
        int zero_crossing_interval_ms = 0;
        if (zero_crossing)
        {
            if (prev_zero_crossing_timestamp != 0)
                zero_crossing_interval_ms = timestamps[i] - prev_zero_crossing_timestamp;
            prev_zero_crossing_timestamp = timestamps[i];
        }
        zero_crossing_intervals_ms.push_back(zero_crossing_interval_ms);

        // Rate estimator
        int confidence_percent = 0;
        rate_estimation = rate_estimator.process(zero_crossing_interval_ms, confidence_percent);
        rate_estimations.push_back(rate_estimation);
        confidence_percents.push_back(confidence_percent);

        // Phase locked loop
        if (zero_crossing)
        {
            phaseLockedLoop.processZeroCrossing(timestamps[i]);
        }
        uint32_t timeToNextPeakMs = phaseLockedLoop.timeToNextPeakMs(timestamps[i]);
        timeToNextPeaksMs.push_back(timeToNextPeakMs);

        // Beat freq
        float beatFreqHz = phaseLockedLoop.getBeatFreqHz();
        beatFreqHzs.push_back(beatFreqHz);
    }

    // // Filter data
    // std::vector<int> filtered_red_led_adc_values = filterHRMData(timestamps, red_led_adc_values);

    // Print file data
    // for (int i = 0; i < timestamps.size(); ++i) {
    //     std::cout << timestamps[i] << " " << red_led_adc_values[i] << std::endl;
    // }

    // Write the data out to a file
    std::ofstream outfile;
    outfile.open("input_data.txt");
    for (int i = 0; i < timestamps.size(); ++i)
    {
        outfile << timestamps[i] << " " << red_led_adc_values[i] << std::endl;
    }
    outfile.close();

    // Write the data out to a file
    outfile.open("filtered_data.txt");
    for (int i = 0; i < timestamps.size(); ++i)
    {
        outfile << timestamps[i] << " " << filtered_red_led_adc_values[i] << " " << zero_crossing_intervals_ms[i] << 
                " " << rate_estimations[i] << " " << confidence_percents[i] << " " << timeToNextPeaksMs[i] <<
                " " << beatFreqHzs[i] << std::endl;
    }
    outfile.close();

    std::cout << latestLogFile << std::endl;
}
