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

    // Check if shutdown initiated
    bool isShutdownRequested()
    {
        return _shutdownInitiated;
    }

    // Handle shutdown
    void shutdown();

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

    // Time to hold power control pin low for shutdown
    static constexpr uint32_t TIME_TO_HOLD_POWER_CTRL_PIN_LOW_MS = 500;

    // VSENSE pin
    int _vsensePin = -1;

    // VSENSE averaging
    SimpleMovingAverage<100> _vsenseAvg;
    uint32_t _sampleCount = 0;

    // Debounce button
    bool _buttonPressed = false;
    uint32_t _buttonPressChangeTimeMs = 0;
    uint32_t _buttonPressDownTimeMs = 0;

    // Off time threshold for button press ms
    static constexpr uint32_t BUTTON_OFF_TIME_THRESHOLD_MS = 2000;

    // Debug
    uint32_t _lastDebugTimeMs = 0;
};
