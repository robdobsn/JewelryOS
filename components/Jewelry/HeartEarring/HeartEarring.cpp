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

// Debug heart rate
// #define DEBUG_HEART_RATE
// #define DEBUG_HEART_RATE_SAMPLES

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Constructor
HeartEarring::HeartEarring()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Destructor
HeartEarring::~HeartEarring()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Setup
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
#ifdef FEATURE_MAX30101_SENSOR
    RaftJsonPrefixed configMAX30101(config, "MAX30101");
    _max30101.setup(configMAX30101, &_i2cCentral);
#endif

    // Set initialized
    _isInitialized = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Loop - called frequently
void HeartEarring::loop()
{
    // Check valid
    if (!_isInitialized)
        return;

#ifdef FEATURE_MAX30101_SENSOR
    // Service MAX30101
    _max30101.loop();
#endif

    // Handle new sensor data
    uint16_t sensorValue = 0;
    uint32_t sampleTimeMs = 0;
    while(_max30101.getSample(sensorValue, sampleTimeMs))
    {
        // Process HRM value
        _hrmAnalysis.process(sensorValue, sampleTimeMs);

#ifdef DEBUG_HEART_RATE_SAMPLES
        // Debug
        LOG_I(MODULE_PREFIX, "loop ms %d red %d lpf %.3f hpf %.3f z %d",
                sampleTimeMs, 
                sensorValue,
                _hrmAnalysis._debugLPFSample,
                _hrmAnalysis._debugHPFSample,
                _hrmAnalysis._debugIsZeroCrossing);
#endif
    }

    // Debug
#ifdef DEBUG_HEART_RATE
    if (Raft::isTimeout(millis(), _lastDebugTimeMs, 1000))
    {
        LOG_I(MODULE_PREFIX, "loop HR %.3fHz (%.3f BPM) timeToNextPeakMs %d interval %dms",
                    _hrmAnalysis.getHeartRateHz(),
                    _hrmAnalysis.getHeartRateHz() * 60,
                    (int)_hrmAnalysis.getTimeToNextPeakMs(millis()),
                    (int)_hrmAnalysis.getHeartRatePulseIntervalMs());
        _lastDebugTimeMs = millis();
    }
#endif

    // Check animation step
    uint64_t timeNowUs = micros();
    uint64_t timeToSleepUs = 0;

    // Check state
    if (_isPulseStart)
    {
        timeToSleepUs = Raft::timeToTimeout(timeNowUs, _timeOfLastStepUs, _timeToNextPulseAnimStartUs);
        if (timeToSleepUs == 0)
        {
            // Start pulse animation
            _timeOfLastStepUs = timeNowUs;
            _ledHeart.startPulseAnimation();
            _isPulseStart = false;
            uint64_t nextStepUs = _ledHeart.getTimeToNextAnimStepUs();
            timeToSleepUs = (nextStepUs == UINT32_MAX) ? 0 : Raft::timeToTimeout(timeNowUs, _timeOfLastStepUs, nextStepUs);
        }
    }
    else
    {
        // Get time to next animation step
        // UINT32_MAX means that animation has finished
        uint64_t nextStepUs = _ledHeart.getTimeToNextAnimStepUs();
        if (nextStepUs == UINT32_MAX)
        {
            _isPulseStart = true;
            _timeOfLastStepUs = timeNowUs;                
            _timeToNextPulseAnimStartUs = _hrmAnalysis.getTimeToNextPeakMs(millis()) * 1000;
            timeToSleepUs = _timeToNextPulseAnimStartUs;
        }
        else
        {
            timeToSleepUs = Raft::timeToTimeout(timeNowUs, _timeOfLastStepUs, nextStepUs);
            if (timeToSleepUs == 0)
            {
                // Handle animation step
                _ledHeart.loop();
                _timeOfLastStepUs = timeNowUs;                
            }
        }
    }

#ifdef FEATURE_ENABLE_SLEEP_MODE
    // Set wakeup timer to worst case time
    esp_sleep_enable_timer_wakeup(timeToSleepUs);

    // If enabled, set to wakeup on GPIO pins (already setup in MAX30101 hardware init)
#ifdef FEATURE_MAX30101_SENSOR    
    if (_max30101.isWakeupOnFifoFullEnabled())
    {
        esp_sleep_enable_gpio_wakeup();
    }
#endif

    // Enter light sleep
    esp_light_sleep_start();
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Shutdown
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



