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

static const char *MODULE_PREFIX = "PowerControl";

PowerControl::PowerControl()
{
}

PowerControl::~PowerControl()
{
}

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
        LOG_I(MODULE_PREFIX, "setup powerCtrlPin %d vSensePin %d currentADC %d currentVoltage %.2fV vsenseSlope %.5f vsenseIntercept %.2f", 
                    _powerCtrlPin, _vsensePin, (int)adcReading, 
                    getVoltageFromADCReading(adcReading), 
                    _vsenseSlope, _vsenseIntercept);
    }
    else
    {
        LOG_I(MODULE_PREFIX, "setup FAILED powerCtrlPin %d vSensePin INVALID vsenseSlope %.5f vsenseIntercept %.2f", 
                    _powerCtrlPin, _vsenseSlope, _vsenseIntercept);
    }

}

void PowerControl::service()
{
    // Update VSENSE average
    if (_vsensePin >= 0)
    {
        // Get VSENSE value
        uint32_t vsenseVal = analogRead(_vsensePin);

        // The pushbutton is wired to VSENSE so if it is pressed the VSENSE pin
        // will go above a threshold
        if (vsenseVal > VSENSE_BUTTON_PRESSED_THRESHOLD)
        {
            // Note time press started
            if (!_buttonPressed)
                _buttonPressDownTimeMs = millis();

            // Pressed
            _buttonPressed = true;
            _buttonPressChangeTimeMs = millis();

            // Check if button press is over the off time threshold
            if (Raft::timeElapsed(millis(), _buttonPressDownTimeMs) > BUTTON_OFF_TIME_THRESHOLD_MS)
            {
                // Debug
                LOG_I(MODULE_PREFIX, "Button pressed for %dms - shutting down",
                        (int)Raft::timeElapsed(millis(), _buttonPressDownTimeMs));
                delay(200);

                // Shutdown initiated
                _shutdownInitiated = true;
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
                    LOG_I(MODULE_PREFIX, "Button pressed for %dms and released",
                            (int)Raft::timeElapsed(millis(), _buttonPressDownTimeMs));
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
    }

    // Debug
    if (Raft::isTimeout(millis(), _lastDebugTimeMs, 1000))
    {
        LOG_I(MODULE_PREFIX, "service %d avg %d voltage %.2fV battLowThreshold %.2fV sampleCount %d",
                    analogRead(_vsensePin), 
                    _vsenseAvg.getAverage(),
                    getVoltageFromADCReading(_vsenseAvg.getAverage()),
                    BATTERY_LOW_THRESHOLD,
                    _sampleCount);
        _lastDebugTimeMs = millis();
    }

    // Check for shutdown due to battery low
    if (!_shutdownInitiated && (_vsensePin >= 0) && (_sampleCount > 100))
    {
        // Get voltage
        float voltage = getVoltageFromADCReading(_vsenseAvg.getAverage());

        // Check for shutdown
        if (voltage < BATTERY_LOW_THRESHOLD)
        {
            // Debug
            LOG_I(MODULE_PREFIX, "Battery low %s voltage %.2fV instADC %d avgADC %d battLowThreshold %.2fV", 
#ifdef FEATURE_SHUTDOWN_DUE_TO_BATTERY_LOW
                    "shutting down",
#else
                    "!!! SHUTDOWN DISABLED !!!",
#endif
                    voltage, analogRead(_vsensePin), _vsenseAvg.getAverage(), BATTERY_LOW_THRESHOLD);
            delay(200);

            // Shutdown initiated
#ifdef FEATURE_SHUTDOWN_DUE_TO_BATTERY_LOW
            _shutdownInitiated = true;
#endif
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Conversion of ADC value to voltage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float PowerControl::getVoltageFromADCReading(uint32_t adcReading)
{
    // Convert to voltage
    return adcReading * _vsenseSlope + _vsenseIntercept;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shutdown
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PowerControl::shutdown()
{
    // Shutdown
    digitalWrite(_powerCtrlPin, LOW);
    delay(TIME_TO_HOLD_POWER_CTRL_PIN_LOW_MS);

    // Enter light sleep with no wakeup
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_light_sleep_start();
}
