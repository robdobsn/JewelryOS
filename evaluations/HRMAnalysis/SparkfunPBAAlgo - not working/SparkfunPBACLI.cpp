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
#include "heartRate.h"

int main(int argc, char **argv)
{
    // Check args
    if (argc <= 2)
    {
        std::cout << "Usage: SparkfunPBACLI <input_filename> <output_filename>" << std::endl;
        return 1;
    }

    // Get the file name from the args
    std::string inData = argv[1];
    std::string outData = argv[2];

    // Read file data
    auto hrmDataRead = readHRMAnalogValues(inData);

    // Output vectors
    std::vector<double> libCalcHeartRateHz;
    std::vector<int> libLastBeatMs;
    std::vector<int> libIIRFilteredData;

    // Run the sparkfun algorithm
    const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
    byte rates[RATE_SIZE]; //Array of heart rates
    byte rateSpot = 0;
    long lastBeat = 0; //Time at which the last beat occurred
    float beatsPerMinute = 0;
    int beatAvg = 0;
    for (int i = 0; i < hrmDataRead.ir_led_adc_values.size(); ++i)
    {
        int irValue = hrmDataRead.ir_led_adc_values[i];
        uint32_t timeMs = hrmDataRead.timestamps[i];
        int16_t iirFilteredData = 0;
        if (checkForBeat(irValue, iirFilteredData, true) == true)
        {
            // We sensed a beat!
            long delta = timeMs - lastBeat;
            lastBeat = timeMs;

            beatsPerMinute = 60 / (delta / 1000.0);

            if (beatsPerMinute < 255 && beatsPerMinute > 20)
            {
                rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
                rateSpot %= RATE_SIZE;                    // Wrap variable

                // Take average of readings
                beatAvg = 0;
                for (byte x = 0; x < RATE_SIZE; x++)
                    beatAvg += rates[x];
                beatAvg /= RATE_SIZE;
            }
        }
        libLastBeatMs.push_back(lastBeat);
        libCalcHeartRateHz.push_back(beatsPerMinute*60);
        libIIRFilteredData.push_back(iirFilteredData);
    }

    // Write the data out to a file
    std::ofstream outfile;
    outfile.open(outData);

    // Write the header
    outfile << "Time (ms),Estimated HR (Hz),Estimated HR (bpm),Last beat (ms),Filtered" << std::endl;
    int estimatedHRIdx = 0;
    for (int i = 0; i < hrmDataRead.timestamps.size(); ++i)
    {
        outfile << hrmDataRead.timestamps[i] << "," << libCalcHeartRateHz[i] << "," << int(libCalcHeartRateHz[i] * 60) << "," << int(libLastBeatMs[i]) << "," << libIIRFilteredData[i] << std::endl;
    }
    outfile.close();

    std::cout << "Output written to " << outData << std::endl;

    return 0;
}
