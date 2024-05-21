/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IIR Filter
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <stdio.h>

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
