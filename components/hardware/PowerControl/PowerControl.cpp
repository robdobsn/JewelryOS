/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Power control
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PowerControl.h"
#include "RaftJsonIF.h"
#include "ConfigPinMap.h"
#include "RaftUtils.h"
#include "esp_sleep.h"

// #define DEBUG_USER_BUTTON_PRESS
// #define DEBUG_POWER_CONTROL
// #define DEBUG_BATTERY_PERCENT

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Constructor
PowerControl::PowerControl()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Destructor
PowerControl::~PowerControl()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Setup
/// @param config
void PowerControl::setup(RaftJsonIF& config)
{
    // Get power control pin
    String pinName = config.getString("powerCtrlPin", "");
    _powerCtrlPin = ConfigPinMap::getPinFromName(pinName.c_str());

    // Set power control pin to ensure power remains on
    if (_powerCtrlPin >= 0)
    {
        // Set power control pin
        pinMode(_powerCtrlPin, OUTPUT);
        digitalWrite(_powerCtrlPin, HIGH);
    }

    // Setup VSENSE pin
    pinName = config.getString("vsensePin", "");
    _vsensePin = ConfigPinMap::getPinFromName(pinName.c_str());
    if (_vsensePin >= 0)
    {
        // Set VSENSE pin
        pinMode(_vsensePin, INPUT);
    }

    // Get battery low voltage
    _batteryLowV = config.getDouble("batteryLowV", BATTERY_LOW_V_DEFAULT);

    // Get VSENSE button level
    _vsenseButtonLevel = config.getLong("vsenseButtonLevel", VSENSE_BUTTON_LEVEL_DEFAULT);

    // Get button off time
    _buttonOffTimeMs = config.getLong("buttonOffTimeMs", 2000);

    // Get ADC calibration
    _vsenseSlope = VSENSE_SLOPE_DEFAULT;
    _vsenseIntercept = VSENSE_INTERCEPT_DEFAULT;
    double v1 = config.getDouble("adcCalib/v1", 0);
    int a1 = config.getLong("adcCalib/a1", 0);
    double v2 = config.getDouble("adcCalib/v2", 0);
    int a2 = config.getLong("adcCalib/a2", 0);
    LOG_I(MODULE_PREFIX, "setup powerCtrlPin %d vSensePin %d v1 %.2f a1 %d v2 %.2f a2 %d", 
                _powerCtrlPin, _vsensePin, v1, a1, v2, a2);

    // If a1 and a2 specified then use them
    if ((a1 > 0) && (a2 > 0))
    {
        _vsenseSlope = (v2 - v1) / (a2 - a1);
        _vsenseIntercept = v1 - _vsenseSlope * a1;
    }

    // Debug
    if (_vsensePin > 0)
    {
        uint32_t adcReading = _vsensePin > 0 ? analogRead(_vsensePin) : 0;
        LOG_I(MODULE_PREFIX, "setup powerCtrlPin %d vSensePin %d currentADC %d currentVoltage %.2fV vsenseSlope %.5f vsenseIntercept %.2f batteryLowV %.2f vSenseButtonLevel %d buttonOffTime %dms", 
                    _powerCtrlPin, _vsensePin, (int)adcReading, 
                    getVoltageFromADCReading(adcReading), 
                    _vsenseSlope, _vsenseIntercept,
                    _batteryLowV, _vsenseButtonLevel, _buttonOffTimeMs);
    }
    else
    {
        LOG_I(MODULE_PREFIX, "setup FAILED powerCtrlPin %d vSensePin INVALID vsenseSlope %.5f vsenseIntercept %.2f", 
                    _powerCtrlPin, _vsenseSlope, _vsenseIntercept);
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Loop
void PowerControl::loop()
{
    // Update VSENSE average
    if (_vsensePin < 0)
        return;

    // Check time for power check
    if (!Raft::isTimeout(millis(), _lastPowerCheckTimeMs, POWER_CHECK_INTERVAL_MS))
        return;

    // Check power
    _lastPowerCheckTimeMs = millis();

    // Get VSENSE value
    uint32_t vsenseVal = analogRead(_vsensePin);

    // Debug
#ifdef DEBUG_POWER_CONTROL
    if (Raft::isTimeout(millis(), _lastDebugTimeMs, 1000))
    {
        LOG_I(MODULE_PREFIX, "loop vSense %d avgVSense %d Vcalculated %.2fV battLowThreshold %.2fV sampleCount %d buttonLevel %d buttonPressed %d",
                    analogRead(_vsensePin), 
                    _vsenseAvg.getAverage(),
                    getVoltageFromADCReading(_vsenseAvg.getAverage()),
                    _batteryLowV,
                    _sampleCount,
                    _vsenseButtonLevel,
                    _buttonPressed);
        _lastDebugTimeMs = millis();
    }
#endif // DEBUG_POWER_CONTROL

    // The pushbutton is wired to VSENSE so if it is pressed the VSENSE pin
    // will go above a threshold
    if (vsenseVal > _vsenseButtonLevel)
    {
        // Note time press started
        if (!_buttonPressed)
            _buttonPressDownTimeMs = millis();

        // Pressed
        _buttonPressed = true;
        _buttonPressChangeTimeMs = millis();

        // Check if button press is over the off time threshold
        if (Raft::timeElapsed(millis(), _buttonPressDownTimeMs) > _buttonOffTimeMs)
        {
            // Debug
#ifdef DEBUG_USER_BUTTON_PRESS
            if (Raft::isTimeout(millis(), _lastWarnUserShutdownTimeMs, 1000))
            {
                LOG_I(MODULE_PREFIX, "loop button pressed for %dms (vsense %d buttonLevel %d buttonOffTime %dms)",
                    (int)Raft::timeElapsed(millis(), _buttonPressDownTimeMs), vsenseVal, _vsenseButtonLevel, _buttonOffTimeMs);
                _lastWarnUserShutdownTimeMs = millis();
            }
#endif // DEBUG_USER_BUTTON_PRESS

            // Shutdown initiated
#ifdef FEATURE_POWER_CONTROL_USER_SHUTDOWN
            _shutdownInitiated = true;
#endif // FEATURE_POWER_CONTROL_USER_SHUTDOWN
        }
    }
    else
    {
        // Not pressed - debounce
        if (_buttonPressed)
        {
            if (Raft::isTimeout(millis(), _buttonPressChangeTimeMs, 200))
            {
                // Button pressed
#ifdef DEBUG_USER_BUTTON_PRESS
                LOG_I(MODULE_PREFIX, "loop button pressed for %dms and released (button off time %dms)",
                        (int)Raft::timeElapsed(millis(), _buttonPressDownTimeMs), _buttonOffTimeMs);
#endif // DEBUG_USER_BUTTON_PRESS
                _buttonPressed = false;
            }
        }
        else
        {
            // Average vsense values that are not button presses
            _vsenseAvg.sample(vsenseVal);
            _sampleCount++;
        }
    }

    // Check for shutdown due to battery low
    if (!_shutdownInitiated && (_sampleCount > 100))
    {
        // Get voltage
        float voltage = getVoltageFromADCReading(_vsenseAvg.getAverage());

        // Get battery percentage
        _batteryPercent = (uint8_t) voltageToPercentage(voltage);
        _batteryPercentValid = true;

#ifdef DEBUG_BATTERY_PERCENT
        LOG_I(MODULE_PREFIX, "loop voltage %.2f batteryPercent %d", voltage, _batteryPercent);
#endif // DEBUG_BATTERY_PERCENT

        // Check for shutdown
        if (voltage < _batteryLowV)
        {
            // Debug
#ifdef DEBUG_POWER_CONTROL
            if (Raft::isTimeout(millis(), _lastWarnBatLowShutdownTimeMs, 1000))
            {
                LOG_I(MODULE_PREFIX, "Battery low %s voltage %.2fV instADC %d avgADC %d battLowThreshold %.2fV", 
#ifdef FEATURE_POWER_CONTROL_LOW_BATTERY_SHUTDOWN
                        "shutting down",
#else
                        "!!! SHUTDOWN DISABLED !!!",
#endif // FEATURE_POWER_CONTROL_LOW_BATTERY_SHUTDOWN
                    voltage, analogRead(_vsensePin), _vsenseAvg.getAverage(), _batteryLowV);
                _lastWarnBatLowShutdownTimeMs = millis();
            }
#endif // DEBUG_POWER_CONTROL

            // Shutdown initiated
#ifdef FEATURE_POWER_CONTROL_LOW_BATTERY_SHUTDOWN
            _shutdownInitiated = true;
#endif // FEATURE_POWER_CONTROL_LOW_BATTERY_SHUTDOWN
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Get battery voltage from ADC reading
/// @param adcReading
/// @return float
float PowerControl::getVoltageFromADCReading(uint32_t adcReading)
{
    // Convert to voltage
    return adcReading * _vsenseSlope + _vsenseIntercept;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Get battery percentage from voltage
/// @param voltage
/// @return int
double PowerControl::voltageToPercentage(double voltage) 
{
    double a = -161.89242958;
    double b = 1774.25221818;
    double c = -6333.43061713;
    double d = 7397.02208126;

    if (voltage > 4.2) return 100;
    if (voltage < 3.3) return 0;
    return a * std::pow(voltage, 3) + b * std::pow(voltage, 2) + c * voltage + d;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Shutdown
void PowerControl::shutdown()
{
    // Shutdown
    digitalWrite(_powerCtrlPin, LOW);
    delay(TIME_TO_HOLD_POWER_CTRL_PIN_LOW_MS);

    // Enter light sleep with no wakeup
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_light_sleep_start();
}
