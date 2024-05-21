#include <iostream>
#include <vector>
#include <cmath>

class IIRFilter4thOrder
{
public:
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

int main() {
    // Coefficients from the Python script
    double a[] = {1.0, -2.99198635, 3.52764744, -1.97218019, 0.45044543};
    double b[] = {0.05644846, 0.0, -0.11289692, 0.0, 0.05644846};
    double zi[] = {-0.05644846, 0.0, 0.05644846, 0.0};

    // Create the filter
    IIRFilter4thOrder filter(a, b, zi);

    // Generate a sample signal for testing
    double fs = 25.0;
    double t = 100.0;
    int N = static_cast<int>(fs * t);
    std::vector<double> data(N);
    for (int i = 0; i < N; i++) {
        data[i] = sin(1.0 * 2 * M_PI * i / fs) + 0.5 * sin(2.0 * 2 * M_PI * i / fs);
    }

    // Apply the filter and print results
    for (int i = 0; i < N; i++) {
        double filtered_value = filter.process(data[i]);
        // if (i >= 0 && i <= 100) {
            printf("x1: %f, y1: %f\n", data[i], filtered_value);
        // }
    }

    return 0;
}
