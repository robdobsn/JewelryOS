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

#include "ReadHRMData.h"
#include "Filters_Eval.h"
#include "ZeroCrossing_Eval.h"
#include "RateEstimator_Eval.h"
#include "PLL_Eval.h"

#include "HRMAnalysis.h"

// #define READ_FROM_HRM_WIFI_LOG_FILE

IIRFilter_Eval lowPassFilter(1, -1.7786317778245846, 0.8008026466657073, 0.005542717210280682, 0.011085434420561363, 0.005542717210280682, 0.9944572827897219, -0.7952599294554288);
IIRFilter_Eval highPassFilter(1, -1.86689228, 0.87521455, 0.93552671, -1.87105341, 0.93552671, -0.93552671, 0.93552671);
ZeroCrossingDetector_Eval zero_crossing_detector;
RateEstimator_Eval rate_estimator(200, 40);
PhaseLockedLoop_Eval phaseLockedLoop(3.5, 0.5, 10);

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
#ifdef READ_FROM_HRM_WIFI_LOG_FILE
    // File name
    std::string latestLogFile = "../HRMReadOverWifi/HRMReadOverWifi.txt";
    std::vector<int> timestamps;
    std::vector<int> red_led_adc_values;
    // Read file data - single column of numbers read into red_led_adc_values
    std::ifstream file(latestLogFile);
    std::string line;
    int time_ms = 0;
    while (std::getline(file, line))
    {
        red_led_adc_values.push_back(std::stoi(line));
        timestamps.push_back(time_ms);
        time_ms += 20;
    }
#else
    std::string logFilePath = "../../logs";
    std::string latestLogFile = findLatestLogFile(logFilePath);
    // Read file data
    auto hrmDataRead = readHRMDataNonArray(latestLogFile);
    // Read heart rates
    auto [heart_rate_timestamps, heart_rates] = readHeartRates(latestLogFile);
#endif

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


    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Calculations done locally

    // Process data sequentially
    std::vector<int> filtered_red_led_adc_values;
    std::vector<int> zero_crossing_intervals_ms;
    std::vector<int> rate_estimations;
    std::vector<int> confidence_percents;
    std::vector<uint32_t> timeToNextPeaksMs;
    std::vector<float> beatFreqHzs;
    int prev_zero_crossing_timestamp = 0;
    int rate_estimation = 0;
    for (int i = 0; i < hrmDataRead.red_led_adc_values.size(); ++i)
    {
        // IIR bandpass filter
        int filtered_sample = lowPassFilter.process(hrmDataRead.red_led_adc_values[i]);
        filtered_sample = highPassFilter.process(filtered_sample);
        filtered_red_led_adc_values.push_back(filtered_sample);

        // Zero crossing detector
        bool zero_crossing = zero_crossing_detector.process(filtered_sample, hrmDataRead.timestamps[i]);
        int zero_crossing_interval_ms = 0;
        if (zero_crossing)
        {
            if (prev_zero_crossing_timestamp != 0)
                zero_crossing_interval_ms = hrmDataRead.timestamps[i] - prev_zero_crossing_timestamp;
            prev_zero_crossing_timestamp = hrmDataRead.timestamps[i];
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
            phaseLockedLoop.processZeroCrossing(hrmDataRead.timestamps[i]);
        }
        uint32_t timeToNextPeakMs = phaseLockedLoop.timeToNextPeakMs(hrmDataRead.timestamps[i]);
        timeToNextPeaksMs.push_back(timeToNextPeakMs);

        // Beat freq
        float beatFreqHz = phaseLockedLoop.getBeatFreqHz();
        beatFreqHzs.push_back(beatFreqHz);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Calculations replicating those done using the library

    HRMAnalysis hrmAnalysis;
    std::vector<double> libCalcHeartRateHz;
    std::vector<int> libCalcTimeToNextPeakMs;
    std::vector<int> libCalcHeartRatePulseIntervalMs;
    for (int i = 0; i < hrmDataRead.red_led_adc_values.size(); ++i)
    {
        hrmAnalysis.process(hrmDataRead.red_led_adc_values[i], hrmDataRead.timestamps[i]);
        libCalcHeartRateHz.push_back(hrmAnalysis.getHeartRateHz());
        libCalcTimeToNextPeakMs.push_back(hrmAnalysis.getTimeToNextPeakMs(hrmDataRead.timestamps[i]));
        libCalcHeartRatePulseIntervalMs.push_back(hrmAnalysis.getHeartRatePulseIntervalMs());
    }

    // // Filter data
    // std::vector<int> filtered_red_led_adc_values = filterHRMData(timestamps, red_led_adc_values);

    // Print file data
    // for (int i = 0; i < timestamps.size(); ++i) {
    //     std::cout << timestamps[i] << " " << red_led_adc_values[i] << std::endl;
    // }

    // Write the input data out to a file
    std::ofstream outfile;
    outfile.open("input_data.txt");
    for (int i = 0; i < hrmDataRead.timestamps.size(); ++i)
    {
        outfile << hrmDataRead.timestamps[i] << " " << hrmDataRead.red_led_adc_values[i] << std::endl;
    }
    outfile.close();

    // Write the data out to a file
    outfile.open("filtered_data.txt");
    int estimatedHR = 0;
    int estimatedHRIdx = 0;
    for (int i = 0; i < hrmDataRead.timestamps.size(); ++i)
    {
        // Find the latest estimated heart rate
        while ((estimatedHRIdx < heart_rate_timestamps.size()) && (heart_rate_timestamps[estimatedHRIdx] < hrmDataRead.timestamps[i]))
        {
            estimatedHR = heart_rates[estimatedHRIdx];
            estimatedHRIdx++;
        }

        outfile << hrmDataRead.timestamps[i] << 
                " " << filtered_red_led_adc_values[i] << 
                " " << zero_crossing_intervals_ms[i] << 
                " " << rate_estimations[i] << 
                " " << confidence_percents[i] << 
                " " << timeToNextPeaksMs[i] <<
                " " << beatFreqHzs[i] << 
                " " << estimatedHR << 
                " " << libCalcHeartRateHz[i] << 
                " " << int(libCalcHeartRateHz[i] * 60) <<
                " " << libCalcTimeToNextPeakMs[i] <<
                " " << libCalcHeartRatePulseIntervalMs[i] <<
                " " << hrmDataRead.lpf[i] << 
                " " << hrmDataRead.hpf[i] << 
                " " << hrmDataRead.zero_crossings[i] <<
                std::endl;
    }
    outfile.close();

    std::cout << latestLogFile << std::endl;
}
