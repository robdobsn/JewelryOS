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
    IIRFilter4thOrder(double* a_coeffs, double* b_coeffs, double* zi_initial)
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
