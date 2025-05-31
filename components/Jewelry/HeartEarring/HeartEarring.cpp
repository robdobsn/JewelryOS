/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Heart Earring
//
// Rob Dobson 2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "HeartEarring.h"
#include "ConfigPinMap.h"
#include "RaftCore.h"
#include "RaftJsonPrefixed.h"
#include "DeviceManager.h"
#include "DevicePollRecords_generated.h"
#include "DeviceTypeRecords.h"
#include "esp_sleep.h"

// Debug heart rate
// #define DEBUG_HEART_RATE
// #define DEBUG_HEART_RATE_SAMPLES
// #define DEBUG_DEVICE_DATA_CALLBACK

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Constructor
HeartEarring::HeartEarring()
{
    // Mutexes
    RaftMutex_init(_heartRateValueMutex);
    RaftMutex_init(_lastSamplesJSONMutex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Destructor
HeartEarring::~HeartEarring()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Setup
void HeartEarring::setup(const RaftJsonIF& config, DeviceManager& devMan)
{
    // Setup LED heart
    RaftJsonPrefixed configLEDHeart(config, "LEDHeart");
    _ledHeart.setup(configLEDHeart);

    // Collect HRM samples
    _collectHRM = config.getBool("collectHRM", false);

    // Register with device manager
    devMan.registerForDeviceData("I2CA_0x57@0", 
        [this](uint32_t deviceTypeIdx, std::vector<uint8_t> data, const void* pCallbackInfo) {

            // Decode device data
            static const uint32_t MAX_ANALOG_READ_SAMPLES = 50;
            poll_MAX30101 deviceData[MAX_ANALOG_READ_SAMPLES];
            DeviceTypeRecordDecodeFn pDecodeFn = deviceTypeRecords.getPollDecodeFn(deviceTypeIdx);
            if (!pDecodeFn)
                return;

            // Decode the data
            uint32_t recsDecoded = pDecodeFn(data.data(), data.size(), &deviceData, sizeof(deviceData), MAX_ANALOG_READ_SAMPLES, _decodeState);

            // Debug
#ifdef DEBUG_DEVICE_DATA_CALLBACK
            LOG_I(MODULE_PREFIX, "deviceDataChangeCB devTypeIdx %d data bytes %d callbackInfo %p recs %d timeMs %d Red %d IR %d",
                    deviceTypeIdx, data.size(), pCallbackInfo, recsDecoded, 
                    deviceData[0].timeMs, deviceData[0].Red, deviceData[0].IR);
#endif

#ifdef DEBUG_HEART_RATE_SAMPLES
            String debugStr;
#endif
            // Process HRM value
            HRMAnalysis::HRMResult analysisResult;
            for (uint32_t i = 0; i < recsDecoded; i++)
            {
                // Process HRM value
                analysisResult = _hrmAnalysis.process(deviceData[i].Red, deviceData[i].timeMs);

#ifdef DEBUG_HEART_RATE_SAMPLES
                // Debug
                debugStr += String(deviceData[i].timeMs) + "," + String(deviceData[i].Red) + "," + String(deviceData[i].IR) + "," + String(_hrmAnalysis._debugFilteredSample) + "," + String(_hrmAnalysis._debugIsZeroCrossing) + ";";
#endif
            }

#ifdef DEBUG_HEART_RATE_SAMPLES
            // Debug
            LOG_I(MODULE_PREFIX, "loop BPM %.3f (%.3fHz) %s",
                    analysisResult.heartRateHz * 60,
                    analysisResult.heartRateHz,
                    debugStr.c_str());
#endif

            // Take the semaphore controlling access to heart rate value
            if (RaftMutex_lock(_heartRateValueMutex, 10))
            {

                // Update the heart rate
                _hrmAnalysisResult = analysisResult;

                // Give back the semaphore
                RaftMutex_unlock(_heartRateValueMutex);
            }

            // Sample collection
            if (_collectHRM)
            {                
                // For sample JSON 
                String irJson, redJson, timeJson;
                irJson.reserve(recsDecoded * 10);
                redJson.reserve(recsDecoded * 10);
                timeJson.reserve(recsDecoded * 10);
                for (uint32_t i = 0; i < recsDecoded; i++)
                {
                    irJson += (i==0 ? "" : ",") + String(deviceData[i].IR);
                    redJson += (i==0 ? "" : ",") + String(deviceData[i].Red);
                    timeJson += (i==0 ? "" : ",") + String(deviceData[i].timeMs);
                }
                if (RaftMutex_lock(_lastSamplesJSONMutex, 2))
                {
                    // Store JSON
                    _lastSamplesJSON = "{\"t\":[" + timeJson + "],\"r\":[" + redJson + "],\"i\":[" + irJson + "]}";

                    // Give back the semaphore
                    RaftMutex_unlock(_lastSamplesJSONMutex);                    
                }
            
#ifdef DEBUG_FIFO_DATA
                LOG_I(MODULE_PREFIX, "loop samples time %s red %s IR %s", timeJson.c_str(), redJson.c_str(), irJson.c_str());
#endif
            }
        },
        50
    );

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

    // Debug
#ifdef DEBUG_HEART_RATE
    if (Raft::isTimeout(millis(), _lastDebugTimeMs, 1000))
    {
        LOG_I(MODULE_PREFIX, "loop HR %.3fHz (%.3f BPM) timeOfNextPeakMs %d interval %dms",
                    _hrmAnalysisResult.heartRateHz,
                    _hrmAnalysisResult.heartRateHz * 60,
                    (int)_hrmAnalysisResult.timeOfNextPeakMs,
                    (int)_hrmAnalysisResult.heartRatePulseIntervalMs);
        _lastDebugTimeMs = millis();
    }
#endif

#ifdef FEATURE_HEART_ANIMATIONS

    // Check animation step
    uint64_t timeNowUs = micros();
    _nextSleepDurationUs = 0;

    // Check state
    if (_isPulseStart)
    {
        _nextSleepDurationUs = Raft::timeToTimeout(timeNowUs, _timeOfLastStepUs, _timeToNextPulseAnimStartUs);
        if (_nextSleepDurationUs == 0)
        {
            // Start pulse animation
            _timeOfLastStepUs = timeNowUs;
            _ledHeart.startPulseAnimation();
            _isPulseStart = false;
            uint64_t nextStepUs = _ledHeart.getTimeToNextAnimStepUs();
            _nextSleepDurationUs = (nextStepUs == UINT32_MAX) ? 0 : Raft::timeToTimeout(timeNowUs, _timeOfLastStepUs, nextStepUs);
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
            uint64_t nextPeakUs = _hrmAnalysisResult.timeOfNextPeakMs * 1000;
            _timeToNextPulseAnimStartUs = nextPeakUs > timeNowUs ? nextPeakUs - timeNowUs : 0;
            _nextSleepDurationUs = _timeToNextPulseAnimStartUs;
        }
        else
        {
            _nextSleepDurationUs = Raft::timeToTimeout(timeNowUs, _timeOfLastStepUs, nextStepUs);
            if (_nextSleepDurationUs == 0)
            {
                // Handle animation step
                _ledHeart.loop();
                _timeOfLastStepUs = timeNowUs;
            }
        }
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Shutdown
void HeartEarring::shutdown()
{
    // Check valid
    if (!_isInitialized)
        return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Get named value
/// @param valueName
/// @param isValid
/// @return double
double HeartEarring::getNamedValue(const char* valueName, bool& isValid)
{
    // Assume heart rate required as that is all we have!
    isValid = false;
    double heartRateBPM = 0;

    // Take the semaphore controlling access to heart rate value
    if (RaftMutex_lock(_heartRateValueMutex, 10))
    {
        // Get the heart rate
        heartRateBPM = _hrmAnalysisResult.heartRateHz * 60;
        isValid = true;

        // Give back the semaphore
        RaftMutex_unlock(_heartRateValueMutex);
    }

    return heartRateBPM;
}


