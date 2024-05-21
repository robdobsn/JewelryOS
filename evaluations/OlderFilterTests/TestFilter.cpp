
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

class IIRFilter
{
public:
    // Constructor for 8th order filter
    IIRFilter(const std::vector<double>& a_coeffs, const std::vector<double>& b_coeffs, const std::vector<double>& zi_initial)
    {
        for (int i = 0; i < 9; i++) {
            a[i] = a_coeffs[i];
            b[i] = b_coeffs[i];
        }
        for (int i = 0; i < 8; i++) {
            zi[i] = zi_initial[i];
            v[i] = zi[i];  // Initialize delayed elements to initial conditions
        }
    }

    ~IIRFilter() {}

    double process(double x1) {
        // Calculate the output using the Direct Form I implementation
        double y1 = b[0] * x1 + v[0];
        for (int i = 0; i < 7; i++) {
            v[i] = b[i + 1] * x1 + v[i + 1] - a[i + 1] * y1;
        }
        v[7] = b[8] * x1 - a[8] * y1;
        return y1;
    }

private:
    double a[9];
    double b[9];
    double zi[8];
    double v[8];
};

std::vector<double> extractColumnFromCSVFile(const std::string& filename, int colIdx) {
    std::vector<double> columnData;
    std::ifstream file(filename);
    std::string line;

    // Check if the file opened successfully
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return columnData;
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        int columnIndex = 0;

        while (std::getline(ss, cell, ',')) {
            if (columnIndex == colIdx) {
                try {
                    double value = std::stod(cell);
                    columnData.push_back(value);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid number in file: " << cell << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cerr << "Number out of range in file: " << cell << std::endl;
                }
            }
            columnIndex++;
        }
    }

    file.close();
    return columnData;
}

int main() {
    // Coefficients from the Python script
    std::vector<double> a = {1.0, -6.05960751, 16.45391545, -26.18801509, 26.74607739, -17.95499326, 7.73746173, -1.9574456, 0.22281157};
    std::vector<double> b = {0.00336282, 0.0, -0.01345126, 0.0, 0.02017689, 0.0, -0.01345126, 0.0, 0.00336282};
    std::vector<double> zi = {-0.00336282, -0.00336282, 0.01008845, 0.01008845, -0.01008845, -0.01008845, 0.00336282, 0.00336282};
    // std::vector<double> zi = {0,0,0,0,0,0,0,0};

    // Create the filter
    IIRFilter filter(a, b, zi);

// #define USE_GENERATED_DATA
#ifdef USE_GENERATED_DATA
    // Generate a sample signal for testing
    double fs = 25.0;
    double t = 1.0;
    int N = static_cast<int>(fs * t);
    std::vector<double> data(N);
    for (int i = 0; i < N; i++) {
        data[i] = sin(1.0 * 2 * M_PI * i / fs) + 0.5 * sin(2.0 * 2 * M_PI * i / fs);
    }
#else
    // Read from CSV file 20240519_1_ADC_Data.csv second column is the Red ADC data
    auto data = extractColumnFromCSVFile("20240519_1_ADC_Data.csv", 1);
    int N = data.size();
#endif

    // Apply the filter
    std::cout << "Filtered data: [ ";
    int col_count = 0;
    for (int i = 0; i < N; i++) {
        double filtered_value = filter.process(data[i]);

        if (i >= 1000 && i <= 2000) {
            printf("x1: %f, y1: %f\n", data[i], filtered_value);
        }
        // std::cout << filtered_value << " ";
        // col_count++;
        // if (col_count == 6) {
        //     std::cout << std::endl;
        //     col_count = 0;
        // }
    }
    std::cout << "]" << std::endl;

    return 0;
}
