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
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "ReadAnalogValues.h"
#include "HRMAnalysis.h"

int main(int argc, char **argv)
{
    // Check args
    if (argc <= 2)
    {
        std::cout << "Usage: HRMAnalysisCPPCLI <input_filename> <output_filename>" << std::endl;
        return 1;
    }

    // Get the file name from the args
    std::string inData = argv[1];
    std::string outData = argv[2];

    // Read file data
    auto hrmDataRead = readHRMAnalogValues(inData);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Calculations based on the HRMAnalysis code in the main project

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

    // Write the data out to a file
    std::ofstream outfile;
    outfile.open(outData);

    // Write the header
    outfile << "Time (ms),Estimated HR (Hz),Estimated HR (bpm),Time to next peak (ms),Heart rate pulse interval (ms),Red LED ADC,IR LED ADC" << std::endl;
    int estimatedHRIdx = 0;
    for (int i = 0; i < hrmDataRead.timestamps.size(); ++i)
    {
        outfile << hrmDataRead.timestamps[i] << 
                "," << libCalcHeartRateHz[i] << 
                "," << int(libCalcHeartRateHz[i] * 60) <<
                "," << libCalcTimeToNextPeakMs[i] <<
                "," << libCalcHeartRatePulseIntervalMs[i] <<
                "," << hrmDataRead.red_led_adc_values[i] <<
                "," << hrmDataRead.ir_led_adc_values[i] <<
                std::endl;
    }
    outfile.close();

    std::cout << "Output written to " << outData << std::endl;

    return 0;
}
