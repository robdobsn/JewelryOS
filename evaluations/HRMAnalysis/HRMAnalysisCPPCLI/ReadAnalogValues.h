#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

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

typedef struct HRMAnalogValues
{
    std::vector<double> timestamps;
    std::vector<double> red_led_adc_values;
    std::vector<double> ir_led_adc_values;
} HRMAnalogValues;

HRMAnalogValues readHRMAnalogValues(std::string logfile)
{
    // Format of header and data line is:
    // Time (s),Red,IR
    // 1249.873,2812238,3092781

    HRMAnalogValues hrm_data_read;

    int first_sample_line_time_ms = 0;

    std::ifstream file(logfile);
    std::string line;

    while (std::getline(file, line))
    {
        std::vector<std::string> words = split(line, ',');

        // Check for header line
        if (words[0].find("Time") != std::string::npos)
        {
            continue;
        }

        // Extract the values
        int line_time_ms = std::stof(words[0]) * 1000;
        double red_led_adc_value = std::stof(words[1]);
        double ir_led_adc_value = std::stof(words[2]);

        // Check for first sample time
        if (first_sample_line_time_ms == 0)
        {
            first_sample_line_time_ms = line_time_ms;
        }

        hrm_data_read.timestamps.push_back(line_time_ms - first_sample_line_time_ms);
        hrm_data_read.red_led_adc_values.push_back(red_led_adc_value);
        hrm_data_read.ir_led_adc_values.push_back(ir_led_adc_value);
    }

    return hrm_data_read;
}

