/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Power control
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PowerControl.h"
#include "ConfigBase.h"
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

void PowerControl::setup(ConfigBase& config, const char* pConfigPrefix)
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
            // Pressed
            _buttonPressed = true;
            _buttonPressedTimeMs = millis();
        }
        else
        {
            // Not pressed - debounce
            if (_buttonPressed)
            {
                if (Raft::isTimeout(millis(), _buttonPressedTimeMs, 200))
                {
                    // Button pressed
                    LOG_I(MODULE_PREFIX, "Button pressed and released");
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
        LOG_I(MODULE_PREFIX, "service %d avg %d voltage %.2fV", 
                    analogRead(_vsensePin), 
                    _vsenseAvg.getAverage(),
                    getVoltageFromADCReading(_vsenseAvg.getAverage()));
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
            LOG_I(MODULE_PREFIX, "Battery low - shutting down");
            delay(200);

            // Shutdown initiated
            _shutdownInitiated = true;

            // Shutdown
            digitalWrite(_powerCtrlPin, LOW);

            // Enter light sleep with no wakeup
            esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
            esp_light_sleep_start();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Conversion of ADC value to voltage
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float PowerControl::getVoltageFromADCReading(uint32_t adcReading)
{
    return (float)adcReading / VSENSE_TO_VOLTAGE;
}
