/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PID Controller
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <algorithm>

// #define DEBUG_PID_CONTROLLER

class PIDControl
{
public:
    PIDControl(double kp, double ki, double kd, double max, double min) :
        _kp(kp), _ki(ki), _kd(kd), _max(max), _min(min)
    {
    }
    ~PIDControl()
    {
    }
    double process(double setPoint, double processVariable, uint32_t timeDeltaMs)
    {
        // Calculate error
        double error = setPoint - processVariable;

        // Time delta in seconds
        if (timeDeltaMs == 0)
            return 0;
        double timeDeltaSecs = timeDeltaMs / 1000.0;

        // Proportional term
        double pOut = _kp * error;

        // Integral term
        _integral += error * timeDeltaSecs;
        _integral = std::clamp(_integral, _min / _ki, _max / _ki);
        double iOut = _ki * _integral;

        // Derivative term
        double derivative = 0;
        if (timeDeltaSecs > 0)
            derivative = (error - _lastError) / timeDeltaSecs;
        double dOut = _kd * derivative;

        // Calculate total output
        double output = pOut + iOut + dOut;

        // Restrict to max/min
        output = std::clamp(output, _min, _max);

        // Save error to previous error
        _lastError = error;

#ifdef DEBUG_PID_CONTROLLER
        printf("PID: SP=%f PV=%f ΔT=%f Err=%f P=%f I=%f D=%f Out=%f Int=%f\n", 
                setPoint, processVariable, timeDeltaSecs, error, pOut, iOut, dOut, output, _integral);
#endif

        return output;
    }
private:
    double _kp;
    double _ki;
    double _kd;
    double _max;
    double _min;
    double _lastError = 0;
    double _integral = 0;
};
