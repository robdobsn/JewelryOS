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

class IIRFilter8thOrder
{
public:
    // Constructor for 8th order filter
    IIRFilter8thOrder(const double* a_coeffs, const double* b_coeffs, const double* zi_initial)
    {
        for (int i = 0; i < 9; i++) {
            a[i] = a_coeffs[i];
            b[i] = b_coeffs[i];
        }
        for (int i = 0; i < 8; i++) {
            zi[i] = zi_initial[i];
            v[i] = zi[i];
        }
    }

    ~IIRFilter8thOrder() {}

    double process(double x1) {
        // Calculate the output using the Direct Form I implementation
        double y1 = b[0] * x1 + v[0];
        for (int i = 0; i < 7; i++) {
            v[i] = b[i + 1] * x1 + v[i + 1] - a[i + 1] * y1;
        }
        v[7] = b[8] * x1 - a[8] * y1;

        // printf("x1: %f, y1: %f\n", x1, y1);
        return y1;
    }       

private:
    double a[9];
    double b[9];
    double zi[8];
    double v[8];
};

