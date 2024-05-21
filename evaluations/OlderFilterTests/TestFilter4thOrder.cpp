
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

class IIRFilter4thOrder
{
public:
    // Constructor for 4th order filter
    // a0 + a1*z^-1 + ... + a4*z^-4 = b0 + b1*z^-1 + ... + b4*z^-4
    IIRFilter4thOrder(const double* a_coeffs, const double* b_coeffs, const double* zi_initial)
    {
        for (int i = 0; i < 5; i++) {
            a[i] = a_coeffs[i];
            b[i] = b_coeffs[i];
        }
        for (int i = 0; i < 4; i++) {
            zi[i] = zi_initial[i];
            vm1[i] = zi_initial[i];  // Initialize delayed elements to initial conditions
        }
    }
    ~IIRFilter4thOrder() {}

    double process(double x1) {
        double y1 = b[0] * x1 + vm1[0];
        for (int i = 0; i < 3; i++) {
            vm[i] = b[i + 1] * x1 + vm1[i + 1] - a[i + 1] * y1;
            vm1[i] = vm[i];
        }
        vm1[3] = b[4] * x1 - a[4] * y1;
        return y1 / a[0];
    }

private:
    double a[5];
    double b[5];
    double zi[4];
    double vm1[4];
    double vm[4];
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
    static constexpr double _butterCoeff4A[] = {1.0, -2.99198635, 3.52764744, -1.97218019, 0.45044543};
    static constexpr double _butterCoeff4B[] = {0.05644846, 0.0, -0.11289692, 0.0, 0.05644846};
    static constexpr double _butterZi[] = {-0.05644846, -0.05644846, 0.05644846, 0.05644846};
    // Create the filter
    IIRFilter4thOrder filter(_butterCoeff4A, _butterCoeff4B, _butterZi);

// #define USE_GENERATED_DATA
#ifdef USE_GENERATED_DATA
    // Generate a sample signal for testing
    double fs = 25.0;
    double t = 100.0;
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

    // Apply the filter and print results
    for (int i = 0; i < N; i++) {
        double filtered_value = filter.process(data[i]);
        // if (i >= 0 && i <= 100) {
            printf("x1: %f, y1: %f\n", data[i], filtered_value);
        // }
    }

    return 0;
}
