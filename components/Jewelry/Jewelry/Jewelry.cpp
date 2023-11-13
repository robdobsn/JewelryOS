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

    // Setup I2C
    _i2cCentral.init(0, _sdaPin, _sclPin, _freq);

    // Setup MAX30101
    _max30101.setup(configGetConfig(), "MAX30101", &_i2cCentral);
}

void Jewelry::service()
{
    // Service MAX30101
    _max30101.service();

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


