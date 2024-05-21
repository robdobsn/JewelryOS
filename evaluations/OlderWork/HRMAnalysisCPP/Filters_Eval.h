#pragma once
#include <stdio.h>
#include <stdint.h>

// class BiquadBPFFilter
// {
// public:
//     BiquadBPFFilter()
//     {
//         // Our data is actually 50Hz and we want centre freequency of 1.5Hz
//         // but the args to calculate_coeffs are integers so the
//         // values for fc and fs are scaled up by a factor of 1000
//         // _bpf.calculate_coeffs(.5, 500, 50000);
//         SO_BPF::tp_coeffs coeffs = {
//             0.21279373587133188,
//             0,
//             -0.21279373587133188,
//             -1.524949463540011,
//             0.5744125282573361
//         };
//         _bpf.set_coefficients(coeffs);

//     }
//     ~BiquadBPFFilter()
//     {
//     }
//     int process(int sample)
//     {
//         return _bpf.process(sample);
//     }

// private:
//     SO_BPF _bpf;
// };

// BiquadBPFFilter bpf;
// SO_BPF bpf;

class IIRFilter_Eval
{
public:
    IIRFilter_Eval(double a0, double a1, double a2, double b0, double b1, double b2, double zi0, double zi1)
    {
        a[0] = a0;
        a[1] = a1;
        a[2] = a2;
        b[0] = b0;
        b[1] = b1;
        b[2] = b2;
        zi[0] = zi0;
        zi[1] = zi1;
        // _bpf.setup
        // // Our data is actually 50Hz and we want centre freequency of 1.5Hz
        // // but the args to calculate_coeffs are integers so the
        // // values for fc and fs are scaled up by a factor of 1000
        // // _bpf.calculate_coeffs(.5, 500, 50000);
        // // _bpf.setup(500, 10, 10);
        // _bpf.setup(50, 2, 2);
        // // _bpf.setup(50, 24);

        // // printf("BPF: %f %f %f %f %f %f\n", _bpf.getA0(), _bpf.getA1(), _bpf.getA2(), _bpf.getB1(), _bpf.getB2(), _bpf.getC0());
        // bhigh: [ 0.93552671 -1.87105341  0.93552671] ahigh: [ 1.         -1.86689228  0.87521455] zihigh: [-0.93552671  0.93552671]


    }
    ~IIRFilter_Eval()
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
        if (count < 100)
        {
            printf("i: %d x1: %f y1: %f v1m1: %f v2m1: %f v1m: %f v2m: %f b: %f %f %f a: %f %f %f\n",
                    count, x1, y1, v1m1, v2m1, v1m, v2m, b[0], b[1], b[2], a[0], a[1], a[2]);
            count++;
        }
        return y1;
    }       

private:
    double a[3] = {1, -1.7786317778245846, 0.8008026466657073};
    double b[3] = {0.005542717210280682, 0.011085434420561363, 0.005542717210280682};
    double zi[2] = {0.9944572827897219, -0.7952599294554288};
    double v1m1 = 0, v2m1 = 0, v1m, v2m;
    bool _isFirst = true;
    uint32_t count = 0;

    // Iir::Butterworth::BandPass<2> _bpf;
};

