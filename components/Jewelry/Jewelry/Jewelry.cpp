///////////////////////////////////////////////////////////////////////////////
//
// Jewelry.cpp
//
// Rob Dobson 2023
//
///////////////////////////////////////////////////////////////////////////////

#include "Jewelry.h"
#include "RaftUtils.h"
#include "RaftJson.h"
#include "HeartEarring.h"
#include "GridEarring.h"
#include "RestAPIEndpointManager.h"
#include "esp_private/esp_clk.h"
#include "esp_sleep.h"

// Debug
// #define DEBUG_MAIN_LOOP

///////////////////////////////////////////////////////////////////////////////
/// @brief Constructor
/// @param pModuleName 
/// @param sysConfig 
Jewelry::Jewelry(const char *pModuleName, RaftJsonIF& sysConfig) :
    RaftSysMod(pModuleName, sysConfig)
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Destructor
Jewelry::~Jewelry()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Setup
void Jewelry::setup()
{
    // Call base class
    RaftSysMod::setup();

#ifdef FEATURE_POWER_CONTROL_SETUP
    // Setup power control
    RaftJsonPrefixed powerControlConfig(configGetConfig(), "PowerControl");
    _powerControl.setup(powerControlConfig);
#endif

#if defined(FEATURE_HEART_JEWELRY)
    // Setup heart earring
    _pJewelry = new HeartEarring();
    RaftJsonPrefixed heartConfig(modConfig(), "HeartEarring");
    DeviceManager* pDevMan = getSysManager()->getDeviceManager();
    if (pDevMan)
        _pJewelry->setup(heartConfig, *pDevMan);
#elif defined(FEATURE_GRID_JEWELRY)
    // Setup grid earring
    _pJewelry = new GridEarring();
    RaftJsonPrefixed gridConfig(modConfig(), "GridEarring");
    DeviceManager* pDevMan = getSysManager()->getDeviceManager();
    if (pDevMan)
        _pJewelry->setup(gridConfig, *pDevMan);
#endif

    // Debug
#ifdef DEBUG_USE_GPIO_PIN_FOR_TIMING
    pinMode(DEBUG_USE_GPIO_PIN_FOR_TIMING, OUTPUT);
    digitalWrite(DEBUG_USE_GPIO_PIN_FOR_TIMING, LOW);
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Loop (called frequently)
void Jewelry::loop()
{
    // Check jewelry valid
    if (_pJewelry)
    {
        // Service jewelry
        _pJewelry->loop();

#ifdef FEATURE_ENABLE_SLEEP_MODE
        // Get sleep duration
        uint32_t timeToSleepUs = _pJewelry->getSleepDurationUs();
        if (timeToSleepUs != 0)
        {

            // Set wakeup timer to worst case time
            esp_sleep_enable_timer_wakeup(timeToSleepUs);

            // If enabled, set to wakeup on GPIO pins (already setup in MAX30101 hardware init)
#ifdef FEATURE_MAX30101_SENSOR    
            if (_pJewelry->wakeupOnGPIO())
            {
                esp_sleep_enable_gpio_wakeup();
            }
#endif

            // Enter light sleep
            esp_light_sleep_start();
        }
#endif

        // Update data collection if enabled
        if (_pJewelry->debugAreSamplesAvailable())
        {
            String samplesJSON = _pJewelry->debugGetLastSamplesJSON();
            if (samplesJSON.length() > 0)
            {
                sysModSendCmdJSON("SamplesJSON", samplesJSON.c_str());
            }
        }
    }

    // Power control loop
    _powerControl.loop();

    // Check for shutdown
    if (_powerControl.isShutdownRequested())
    {
        // Debug
        LOG_I(MODULE_PREFIX, "loop shutdown requested");
        delay(100);

        // Shutdown
        if (_pJewelry)
            _pJewelry->shutdown();

        // Shutdown
        _powerControl.shutdown();
    }

#ifdef DEBUG_MAIN_LOOP

    if (Raft::isTimeout(millis(), _lastDebugTimeMs, 1000))
    {
        // Debug
        LOG_I(MODULE_PREFIX, "loop cpu freq %d", esp_clk_cpu_freq());
        _lastDebugTimeMs = millis();
    }

#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Add REST API endpoints
/// @param endpointManager
void Jewelry::addRestAPIEndpoints(RestAPIEndpointManager &endpointManager)
{
    // Control shade
    endpointManager.addEndpoint("jewelry", RestAPIEndpoint::ENDPOINT_CALLBACK, RestAPIEndpoint::ENDPOINT_GET,
                            std::bind(&Jewelry::apiControl, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                            "control jewelry");
    LOG_I(MODULE_PREFIX, "addRestAPIEndpoints jewelry");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief API control
/// @param reqStr
/// @param respStr
/// @param sourceInfo
/// @return RaftRetCode
RaftRetCode Jewelry::apiControl(const String &reqStr, String &respStr, const APISourceInfo& sourceInfo)
{
    // Extract parameters
    std::vector<String> params;
    std::vector<RaftJson::NameValuePair> nameValues;
    RestAPIEndpointManager::getParamsAndNameValues(reqStr.c_str(), params, nameValues);
    RaftJson nameValueParamsJson = RaftJson::getJSONFromNVPairs(nameValues, true);

    // Debug
    LOG_I(MODULE_PREFIX, "apiControl %s", reqStr.c_str());

    // Check for grid setting
    bool rslt = false;
    if (params.size() > 3)
    {
        if (params[1].equalsIgnoreCase("grid"))
        {
            if (params[2].equalsIgnoreCase("msg"))
            {
                LOG_I(MODULE_PREFIX, "apiControl grid msg %s", params[3].c_str());
                rslt = true;
            }
        }
    }
    
    return Raft::setJsonBoolResult(reqStr.c_str(), respStr, rslt);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Get status JSON
/// @return String
String Jewelry::getStatusJSON() const
{
    return "{}";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Get named value
/// @param valueName
/// @param isValid
/// @return double
double Jewelry::getNamedValue(const char* valueName, bool& isValid)
{
    isValid = true;
    if (String(valueName).equalsIgnoreCase("batteryPC"))
    {
        return _powerControl.getBatteryPercent(isValid);
    }
    else if (String(valueName).equalsIgnoreCase("heartRate"))
    {
        return _pJewelry->getNamedValue(valueName, isValid);
    }
    isValid = false;
    return 0;
}