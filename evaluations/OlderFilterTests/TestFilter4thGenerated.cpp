
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
            zi[i] = zi_initial[i];
        }
    }
    ~IIRFilter4thOrder() {}

    double process(double x1) {
        if (_isFirst) {
            for (int i = 0; i < 5; i++) {
                vm1[i] = zi[i] * x1;
            }
            _isFirst = false;
        }
        double y1 = (b[0] * x1 + vm1[0]) / a[0];
        for (int i = 0; i < 4; i++) {
            vm[i] = b[i+1] * x1 + vm1[i+1] - a[i+1] * y1;
            vm1[i] = vm[i];
        }
        vm1[4] = b[4] * x1 - a[4] * y1;
        return y1;
    }       

private:
    double a[5] = {1, 1, 1, 1, 1};
    double b[5] = {1, 1, 1, 1, 1};
    double zi[5] = {1, 1, 1, 1, 1};
    double vm1[5] = {0, 0, 0, 0, 0};
    double vm[5] = {0, 0, 0, 0, 0};
    bool _isFirst = true;
    uint32_t count = 0;
};

int main() {
    // Coefficients from the Python script
    static constexpr double _butterCoeff4A[] = {1.0, -2.99198635, 3.52764744, -1.97218019, 0.45044543};
    static constexpr double _butterCoeff4B[] = {0.05644846, 0.0, -0.11289692, 0.0, 0.05644846};
    static constexpr double _butterZi[] = {0,0,0,0};
    // Create the filter
    IIRFilter4thOrder filter(_butterCoeff4A, _butterCoeff4B, _butterZi);

    // Generate a sample signal for testing
    double fs = 25.0;
    double t = 10.0;
    int N = static_cast<int>(fs * t);
    std::vector<double> data(N);
    for (int i = 0; i < N; i++) {
        data[i] = sin(1.0 * 2 * M_PI * i / fs) + 0.5 * sin(2.0 * 2 * M_PI * i / fs);
    }

    for (int i = 0; i < N; i++) {
        double filtered_value = filter.process(data[i]);

        if (i >= 0 && i <= 100) {
            printf("x1: %f, y1: %f\n", data[i], filtered_value);
        }
    }
    return 0;
}
