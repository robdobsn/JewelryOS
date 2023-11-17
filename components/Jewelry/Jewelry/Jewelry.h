/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Jewelry
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SysModBase.h"
#include "MAX30101.h"
// #include "RaftI2CCentral.h"
#include "BusI2CESPIDF.h"
#include "PowerControl.h"
#include "LEDHeart.h"
#include "HRMAnalysis.h"

class Jewelry : public SysModBase
{
public:
    Jewelry(const char *pModuleName, ConfigBase &defaultConfig, ConfigBase *pGlobalConfig, ConfigBase *pMutableConfig);
    ~Jewelry() override;

protected:

    // Setup
    virtual void setup() override final;

    // Service (called frequently)
    virtual void service() override final;

private:

    // Time between heart pulses
    uint32_t _timeBetweenHeartPulsesMs = 1000;
    uint32_t _lastHeartPulseTimeMs = 0;

    // Debug
    uint32_t _lastDebugTimeMs = 0;

    // MAX30101
    MAX30101 _max30101;

    // I2C
    // RaftI2CCentral _i2cCentral;
    BusI2CESPIDF _i2cCentral;
    int _sdaPin = -1;
    int _sclPin = -1;
    int _freq = 100000;

    // Power control
    PowerControl _powerControl;

    // LED heart display
    LEDHeart _ledHeart;

    // HRM analysis
    HRMAnalysis _hrmAnalysis;
};
