/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Heart Earring
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "HeartEarring.h"
#include "ConfigPinMap.h"
#include "RaftJsonPrefixed.h"
#include "esp_sleep.h"

static const char *MODULE_PREFIX = "HeartEarring";

// The following allows disabling of animations
#define ENABLE_ANIMATIONS

// The following allows disabling sleep
#define ENABLE_SLEEP_MODE

// Debug heart rate
// #define DEBUG_HEART_RATE

HeartEarring::HeartEarring()
{
}

HeartEarring::~HeartEarring()
{
}

void HeartEarring::setup(const RaftJsonIF& config)
{
    // Get I2C details
    String pinName = config.getString("sdaPin", "");
    _sdaPin = ConfigPinMap::getPinFromName(pinName.c_str());
    pinName = config.getString("sclPin", "");
    _sclPin = ConfigPinMap::getPinFromName(pinName.c_str());
    _freq = config.getLong("i2cFreq", 100000);

    // Check valid
    if ((_sdaPin < 0) || (_sclPin < 0))
    {
        LOG_E(MODULE_PREFIX, "setup I2C pins not specified");
        return;
    }

    // Setup LED heart
    RaftJsonPrefixed configLEDHeart(config, "LEDHeart");
    _ledHeart.setup(configLEDHeart);

    // Setup I2C
    bool i2cOk = _i2cCentral.init(0, _sdaPin, _sclPin, _freq);

    // Debug
    LOG_I(MODULE_PREFIX, "setup %s sdaPin %d sclPin %d freq %d", 
                i2cOk ? "OK" : "FAILED", _sdaPin, _sclPin, _freq);

    // Setup MAX30101
    RaftJsonPrefixed configMAX30101(config, "MAX30101");
    _max30101.setup(configMAX30101, &_i2cCentral);

    // Set initialized
    _isInitialized = true;
}

void HeartEarring::service()
{
    // Check valid
    if (!_isInitialized)
        return;

    // Service MAX30101
    _max30101.service();

#ifdef ENABLE_ANIMATIONS
    // Handle heart animation
#ifndef ENABLE_SLEEP_MODE
    bool heartAnimationRequired = Raft::isTimeout(millis(), _lastHeartPulseTimeMs, 
                _hrmAnalysis.getHeartRatePulseIntervalMs());
    if (heartAnimationRequired)
#endif
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

#ifdef ENABLE_SLEEP_MODE
            // Sleep for time to next animation step
            esp_sleep_enable_timer_wakeup(timeToNextAnimStepUs);
            esp_light_sleep_start();
#else // ENABLE_SLEEP_MODE
            // Delay for time to next animation step
#ifdef DEBUG_USE_GPIO_PIN_FOR_TIMING
            digitalWrite(DEBUG_USE_GPIO_PIN_FOR_TIMING, HIGH);
#endif
            delayMicroseconds(timeToNextAnimStepUs);
#ifdef DEBUG_USE_GPIO_PIN_FOR_TIMING
            digitalWrite(DEBUG_USE_GPIO_PIN_FOR_TIMING, LOW);
#endif
#endif // ENABLE_SLEEP_MODE
        }
    }
#endif

    // Check if wakeup on HRM FIFO full is enabled
    if (_max30101.isWakeupOnFifoFullEnabled())
    {
#ifdef ENABLE_SLEEP_MODE
        // Set wakeup timer to worst case time
        esp_sleep_enable_timer_wakeup(_hrmAnalysis.getTimeToNextPeakMs(millis()) * 1000);

        // Set to wakeup on GPIO pins (already setup in MAX30101 hardware init)
        esp_sleep_enable_gpio_wakeup();
        esp_light_sleep_start();
#endif
    }

    // Handle new HRM data
    uint16_t hrmValue = 0;
    uint32_t sampleTimeMs = 0;
    while(_max30101.getSample(hrmValue, sampleTimeMs))
    {
        // Debug
        // LOG_I(MODULE_PREFIX, "service hrmValue %d timeMs %d", hrmValue, sampleTimeMs);

        // Process HRM value
        _hrmAnalysis.process(hrmValue, sampleTimeMs);
    }

    // Debug
#ifdef DEBUG_HEART_RATE
    if (Raft::isTimeout(millis(), _lastDebugTimeMs, 1000))
    {
        LOG_I(MODULE_PREFIX, "service HR %.3fHz (%.3f BPM) timeToNextPeakMs %d interval %dms",
                    _hrmAnalysis.getHeartRateHz(),
                    _hrmAnalysis.getHeartRateHz() * 60,
                    (int)_hrmAnalysis.getTimeToNextPeakMs(millis()),
                    (int)_hrmAnalysis.getHeartRatePulseIntervalMs());
        _lastDebugTimeMs = millis();
    }
#endif
}

void HeartEarring::shutdown()
{
    // Check valid
    if (!_isInitialized)
        return;

    // Shutdown MAX30101
    _max30101.shutdown();

    // Turn off I2C
    _i2cCentral.deinit();
}



