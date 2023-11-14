///////////////////////////////////////////////////////////////////////////////
//
// Jewelry.cpp
//
// Rob Dobson 2023
//
///////////////////////////////////////////////////////////////////////////////

#include "Jewelry.h"
#include "RaftUtils.h"
#include "ConfigPinMap.h"
#include "esp_sleep.h"

static const char *MODULE_PREFIX = "Jewelry";

// TODO TURN OFF
#define USE_PIN_FOR_DEBUG 9

Jewelry::Jewelry(const char *pModuleName, ConfigBase &defaultConfig, ConfigBase *pGlobalConfig, ConfigBase *pMutableConfig) :
    SysModBase(pModuleName, defaultConfig, pGlobalConfig, pMutableConfig)
{
}

Jewelry::~Jewelry()
{
}

void Jewelry::setup()
{
    // Call base class
    SysModBase::setup();

    // Get I2C details
    String pinName = configGetString("sdaPin", "");
    _sdaPin = ConfigPinMap::getPinFromName(pinName.c_str());
    pinName = configGetString("sclPin", "");
    _sclPin = ConfigPinMap::getPinFromName(pinName.c_str());
    _freq = configGetLong("i2cFreq", 100000);

    // Check valid
    if ((_sdaPin < 0) || (_sclPin < 0))
    {
        LOG_E(MODULE_PREFIX, "setup I2C pins not specified");
        return;
    }

    // // Setup power control
    // _powerControl.setup(configGetConfig(), "PowerControl");

    // Setup LED heart
    _ledHeart.setup(configGetConfig(), "LEDHeart");

    // Setup I2C
    _i2cCentral.init(0, _sdaPin, _sclPin, _freq);

    // Setup MAX30101
    _max30101.setup(configGetConfig(), "MAX30101", &_i2cCentral);

    // Debug
#ifdef USE_PIN_FOR_DEBUG
    pinMode(USE_PIN_FOR_DEBUG, OUTPUT);
    digitalWrite(USE_PIN_FOR_DEBUG, LOW);
#endif
}

void Jewelry::service()
{
    // Service MAX30101
    _max30101.service();

    // Handle heart animation
    bool heartAnimationRequired = Raft::isTimeout(millis(), _lastHeartPulseTimeMs, _timeBetweenHeartPulsesMs);
    if (heartAnimationRequired)
    {
        // Start pulse animation
        _ledHeart.startPulseAnimation();
        _lastHeartPulseTimeMs = millis();
        while (true)
        {
            // Service heart LEDs
            _ledHeart.service();

            // Get time to next animation step
            // UINT32_MAX means that animation has finished
            uint32_t timeToNextAnimStepUs = _ledHeart.getTimeToNextAnimStepUs();
            if (timeToNextAnimStepUs == UINT32_MAX)
                break;

            // TODO - change this to light sleep
            // Delay for time to next animation step
    
#ifdef USE_PIN_FOR_DEBUG
            digitalWrite(USE_PIN_FOR_DEBUG, HIGH);
#endif
            delayMicroseconds(timeToNextAnimStepUs);
#ifdef USE_PIN_FOR_DEBUG
            digitalWrite(USE_PIN_FOR_DEBUG, LOW);
#endif
        }
    }

    // // Service power control
    // _powerControl.service();

    // TODO remove

    if (Raft::isTimeout(millis(), _lastDebugTimeMs, 1000))
    {
        // LOG_I(MODULE_PREFIX, "pulseRate %f", _max30101.getPulseRate());
        // LOG_I(MODULE_PREFIX, "Going to sleep .....");
        // delay(1000);
        // esp_light_sleep_start();
        _lastDebugTimeMs = millis();
    }
}


