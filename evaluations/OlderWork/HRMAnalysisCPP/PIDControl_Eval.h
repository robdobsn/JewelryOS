#pragma once

#include <stdio.h>
#include <stdint.h>

class PIDControl_Eval
{
public:
    PIDControl_Eval(float kp, float ki, float kd, float max, float min)
    {
        _kp = kp;
        _ki = ki;
        _kd = kd;
        _max = max;
        _min = min;
    }
    ~PIDControl_Eval()
    {
    }
    float process(float setPoint, float processVariable, uint32_t timeDeltaMs)
    {
        // Calculate error
        float error = setPoint - processVariable;

        // Time delta in seconds
        float timeDeltaSecs = timeDeltaMs / 1000.0;
        if (timeDeltaSecs <= 0)
            return 0;

        // Proportional term
        float pOut = _kp * error;

        // Integral term
        if (timeDeltaSecs > 0)
            _integral += error * timeDeltaSecs;
        float iOut = _ki * _integral;

        // Derivative term
        float derivative = 0;
        if (timeDeltaSecs > 0)
            derivative = (error - _lastError) / timeDeltaSecs;
        float dOut = _kd * derivative;

        // Calculate total output
        float output = pOut + iOut + dOut;

        // Restrict to max/min
        if (output > _max)
            output = _max;
        else if (output < _min)
            output = _min;

        // Save error to previous error
        _lastError = error;

        printf("PID: %f %f %f %f %f %f %f %f %f", setPoint, processVariable, timeDeltaSecs, error, pOut, iOut, dOut, output, _integral);

        return output;
    }
private:
    float _kp;
    float _ki;
    float _kd;
    float _max;
    float _min;
    float _lastError = 0;
    float _integral = 0;
};
