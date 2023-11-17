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

// #define DEBUG_IIR_FILTER_UPTO_POINT 100

class IIRFilter
{
public:
    // Constructor
    // a0 + a1*z^-1 + a2*z^-2 = b0 + b1*z^-1 + b2*z^-2
    // zi0 and zi1 are the initial conditions
    IIRFilter(double a0, double a1, double a2, double b0, double b1, double b2, double zi0, double zi1)
    {
        a[0] = a0;
        a[1] = a1;
        a[2] = a2;
        b[0] = b0;
        b[1] = b1;
        b[2] = b2;
        zi[0] = zi0;
        zi[1] = zi1;
    }
    ~IIRFilter()
    {
    }
    double process(double x1) {
        if (_isFirst)
        {
            v1m1 = zi[0] * x1;
            v2m1 = zi[1] * x1;
            _isFirst = false;
        }
        double y1 = 0;
        y1 = (b[0] * x1 + v1m1) / a[0];
        v1m = (b[1] * x1 + v2m1) - a[1] * y1;
        v2m = b[2] * x1 - a[2] * y1;
        v1m1 = v1m;
        v2m1 = v2m;
#ifdef DEBUG_IIR_FILTER_UPTO_POINT
        if (count < DEBUG_IIR_FILTER_UPTO_POINT)
        {
            printf("i: %d x1: %f y1: %f v1m1: %f v2m1: %f v1m: %f v2m: %f b: %f %f %f a: %f %f %f\n",
                    count, x1, y1, v1m1, v2m1, v1m, v2m, b[0], b[1], b[2], a[0], a[1], a[2]);
            count++;
        }
#endif
        return y1;
    }       

private:
    double a[3] = {1, 1, 1};
    double b[3] = {1, 1, 1};
    double zi[2] = {1, 1};
    double v1m1 = 0, v2m1 = 0, v1m, v2m;
    bool _isFirst = true;
    uint32_t count = 0;
};
