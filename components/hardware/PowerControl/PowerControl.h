/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Power control
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <SimpleMovingAverage.h>

class ConfigBase;

class PowerControl
{
public:
    PowerControl();
    virtual ~PowerControl();

    // Setup
    void setup(ConfigBase& config, const char* pConfigPrefix);

    // Service
    void service();

    // Get voltage
    float getVoltageFromADCReading(uint32_t adcReading);

private:

    // VSENSE threshold when button is pressed
    static const uint32_t VSENSE_BUTTON_PRESSED_THRESHOLD = 2300;

    // VSENSE to voltage conversion
    // Measurements from multimeter
    // 3.584V = 1600
    // 3.697V = 1659
    // 3.804V = 1704
    // 3.899V = 1748
    // 4.003V = 1790
    // 4.107V = 1845
    // 4.203V = 1887
    // Using excel to fit a curve with 0 intercept ...
    static constexpr float VSENSE_TO_VOLTAGE = 448.5;

    // Shutdown due to battery low threshold
    static constexpr float BATTERY_LOW_THRESHOLD = 3.55;

    // Shutdown initiated
    bool _shutdownInitiated = false;
    
    // Power control pin
    int _powerCtrlPin = -1;

    // VSENSE pin
    int _vsensePin = -1;

    // VSENSE averaging
    SimpleMovingAverage<100> _vsenseAvg;
    uint32_t _sampleCount = 0;

    // Debounce button
    bool _buttonPressed = false;
    uint32_t _buttonPressedTimeMs = 0;

    // Debug
    uint32_t _lastDebugTimeMs = 0;
};
