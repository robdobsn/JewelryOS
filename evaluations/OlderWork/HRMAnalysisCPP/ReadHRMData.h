#pragma once

#include <string>
#include <vector>
#include <fstream>

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

typedef struct HRMDataRead
{
    std::vector<int> timestamps;
    std::vector<int> red_led_adc_values;
    std::vector<int> lpf;
    std::vector<int> hpf;
    std::vector<int> zero_crossings;
} HRMDataRead;

HRMDataRead readHRMDataNonArray(std::string logfile)
{
    // Format of line is:
    // I (43370) HeartEarring: service ms 42710 red 27935 lpf 27905.841 hpf 233.139 z 0

    HRMDataRead hrm_data_read;

    int first_sample_line_time_ms = 0;

    std::ifstream file(logfile);
    std::string line;

    while (std::getline(file, line))
    {
        if (line.find("service ms") == std::string::npos)
        {
            continue;
        }

        std::vector<std::string> words = split(line, ' ');

        int line_time_ms = std::stoi(words[5]);
        int red_led_adc_value = std::stoi(words[7]);
        float lpf = std::stof(words[9]);
        float hpf = std::stof(words[11]);
        int is_zero_crossing = std::stoi(words[13]);

        // Check for first sample time
        if (first_sample_line_time_ms == 0)
        {
            first_sample_line_time_ms = line_time_ms;
        }

        hrm_data_read.timestamps.push_back(line_time_ms - first_sample_line_time_ms);
        hrm_data_read.red_led_adc_values.push_back(red_led_adc_value);
        hrm_data_read.lpf.push_back(lpf);
        hrm_data_read.hpf.push_back(hpf);
        hrm_data_read.zero_crossings.push_back(is_zero_crossing);
        
    }

    return hrm_data_read;
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

        // Format of line is
        // I (47510) MAX30101: service i2cAddr 0x57 sampleRate 12.5/s numSamples 7 writePtr 28 readPtr 21 samples {"s":[11286,11299,11295,11305,11308,11313,11295,11299,11299,11299,11287,11278,11284,11290,11279,11273,11277,11298,11356,11292,11297,11317,11309,11476,11295,11300,11252,11294,11286,11290,11287]}

        // Extract the samples JSON
        std::string samples_json = *(it + 1);

        // Split the samples JSON into individual samples
        std::vector<std::string> samples = split(samples_json.substr(6, samples_json.size() - 7), ',');

        // Check for first sample time        
        if (first_sample_line_time_ms == 0)
        {
            first_sample_line_time_ms = line_time_ms;
        }
        int line_start_ms = line_time_ms - first_sample_line_time_ms;
        for (int i = 0; i < samples.size(); ++i)
        {
            int sample_value = std::stoi(samples[i]);
            timestamps.push_back(line_start_ms + (i / sample_rate) * 1000);
            red_led_adc_values.push_back(sample_value);
        }
    }

    return {timestamps, red_led_adc_values};
}

// Read the estimated heart rate from the log file
// I (134443) HeartEarring: service HR 1.037Hz (62.196 BPM) timeToNextPeakMs 602 interval 964ms
std::pair<std::vector<int>, std::vector<int>> readHeartRates(std::string logfile)
{
    std::vector<int> timestamps;
    std::vector<int> heart_rates;
    std::ifstream file(logfile);
    std::string line;

    while (std::getline(file, line))
    {
        if (line.find("service HR") == std::string::npos)
        {
            continue;
        }

        std::vector<std::string> words = split(line, ' ');

        int line_time_ms = std::stoi(words[1].substr(1, words[1].size() - 2));
        int heart_rate = std::stof(words[5]) * 60;

        timestamps.push_back(line_time_ms);
        heart_rates.push_back(heart_rate);
    }

    return {timestamps, heart_rates};
}