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
#include <RestAPIEndpointManager.h>
#include "esp_private/esp_clk.h"

static const char *MODULE_PREFIX = "Jewelry";

// Debug
// #define DEBUG_MAIN_LOOP

///////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
///////////////////////////////////////////////////////////////////////////////

Jewelry::Jewelry(const char *pModuleName, RaftJsonIF& sysConfig) :
    RaftSysMod(pModuleName, sysConfig)
{
}

Jewelry::~Jewelry()
{
}

///////////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////////

void Jewelry::setup()
{
    // Call base class
    RaftSysMod::setup();

#ifdef FEATURE_POWER_CONTROL_SETUP
    // Setup power control
    RaftJsonPrefixed powerControlConfig(configGetConfig(), "PowerControl");
    _powerControl.setup(powerControlConfig);
#endif

    // Check hardware type
    String hwTypeStr = configGetString("hardwareType", "");

    LOG_I(MODULE_PREFIX, "setup hardwareType %s config %s", hwTypeStr.c_str(), modConfig().getJsonDoc());

    if (hwTypeStr == "heart")
    {
        // Setup heart earring
        _pJewelry = new HeartEarring();
        RaftJsonPrefixed heartConfig(modConfig(), "HeartEarring");
        _pJewelry->setup(heartConfig);
    }
    else if (hwTypeStr == "grid")
    {
        // Setup grid earring
        _pJewelry = new GridEarring();
        RaftJsonPrefixed gridConfig(modConfig(), "GridEarring");
        _pJewelry->setup(gridConfig);
    }

    // Debug
#ifdef DEBUG_USE_GPIO_PIN_FOR_TIMING
    pinMode(DEBUG_USE_GPIO_PIN_FOR_TIMING, OUTPUT);
    digitalWrite(DEBUG_USE_GPIO_PIN_FOR_TIMING, LOW);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Loop
///////////////////////////////////////////////////////////////////////////////

void Jewelry::loop()
{
    // Check jewelry valid
    if (_pJewelry)
    {
        // Service jewelry
        _pJewelry->loop();

        // Update data collection if enabled
        String samplesJSON = _pJewelry->getLastSamplesJSON();
        if (samplesJSON.length() > 0)
        {
            sysModSendCmdJSON("SamplesJSON", samplesJSON.c_str());
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
// Endpoints
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Jewelry::addRestAPIEndpoints(RestAPIEndpointManager &endpointManager)
{
    // Control shade
    endpointManager.addEndpoint("jewelry", RestAPIEndpoint::ENDPOINT_CALLBACK, RestAPIEndpoint::ENDPOINT_GET,
                            std::bind(&Jewelry::apiControl, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                            "control jewelry");
    LOG_I(MODULE_PREFIX, "addRestAPIEndpoints jewelry");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Control via API
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
// Get JSON status
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

String Jewelry::getStatusJSON() const
{
    return "{}";
}
