/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Main entry point
//
// JewelOS
// Rob Dobson 2024
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "RaftCoreApp.h"
#include "RegisterSysMods.h"
#include "DetectHardware.h"
#include "Jewelry.h"

// Entry point
extern "C" void app_main(void)
{
    RaftCoreApp raftCoreApp;

    // Detect hardware
    DetectHardware::detectHardware(raftCoreApp);

    // Register SysMods from RaftSysMods library
    RegisterSysMods::registerSysMods(raftCoreApp.getSysManager());

    // Jewelry
    raftCoreApp.registerSysMod("Jewelry", Jewelry::create, true);

    // Loop forever
    while (1)
    {
        // Yield for 1 tick
        vTaskDelay(1);

        // Service the app
        raftCoreApp.loop();
    }
}

#ifdef USE_ORIGINAL_MAIN

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System Name and Version
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MACRO_STRINGIFY(x) #x
#define MACRO_TOSTRING(x) MACRO_STRINGIFY(x)
#define SYSTEM_NAME MACRO_TOSTRING(HW_SYSTEM_NAME)
#define DEFAULT_FRIENDLY_NAME MACRO_TOSTRING(HW_DEFAULT_FRIENDLY_NAME)
#define DEFAULT_HOSTNAME MACRO_TOSTRING(HW_DEFAULT_HOSTNAME)
#define DEFAULT_ADVNAME MACRO_TOSTRING(HW_DEFAULT_ADVNAME)
#define DEFAULT_SERIAL_SET_MAGIC_STR MACRO_TOSTRING(HW_SERIAL_SET_MAGIC_STR)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Default system config
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE:
// In VSCode C++ raw strings can be removed - to reveal JSON with regex search and replace:
//      regex:    R"\((.*)\)"
//      replace:  $1
// And then reinserted with:
//      regex:    (\s*)(.*)
//      replace:  $1R"($2)"

static const char *defaultConfigJSON =
    R"({)"
        R"("SystemName":")" SYSTEM_NAME R"(",)"
        R"("SystemVersion":")" SYSTEM_VERSION R"(",)"
        R"("IDFVersion":")" IDF_VER R"(",)"
        R"("DefaultName":")" DEFAULT_FRIENDLY_NAME R"(",)"
        R"("SysManager":{)"
            R"("monitorPeriodMs":10000,)"
            R"("reportList":["NetMan","BLEMan","SysMan","StatsCB"],)"
            R"("pauseWiFiforBLE":1,)"
            R"("RICSerial":{)"
                R"("FrameBound":"0xE7",)"
                R"("CtrlEscape":"0xD7")"
            R"(})"
        R"(})"
    R"(})"
;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System Parameters
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Main task parameters
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Debug
static const char* MODULE_NAME = "MainTask";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test and Monitoring
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// #define TEST_MINIMAL_FLASH_HEAP_BASE_CASE
// #define DEBUG_SHOW_TASK_INFO
// #define DEBUG_SHOW_RUNTIME_STATS
// #define DEBUG_HEAP_ALLOCATION
// #define DEBUG_TIMING_OF_STARTUP
// #define DEBUG_SHOW_NVS_INFO
// #define DEBUG_SHOW_NVS_CONTENTS

#ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
#ifdef DEBUG_SHOW_TASK_INFO
#define DEBUG_SHOW_TASK_MS 10000
int tasks_info();
uint32_t lasttaskdumpms = 0;
#endif
#ifdef DEBUG_SHOW_RUNTIME_STATS
#define DEBUG_SHOW_RUNTIME_STATS_MS 10000
int runtime_stats();
uint32_t laststatsdumpms = 0;
#endif
#endif

#ifdef DEBUG_HEAP_ALLOCATION
#include "esp_heap_trace.h"
#define NUM_RECORDS 100
static heap_trace_record_t trace_record[NUM_RECORDS]; // This buffer must be in internal RAM
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// C, ESP32 and RTOS
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_task_wdt.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"

// Jewelry specific
#include "Jewelry.h"

// App
#include "SysTypeManager.h"
#include "SysManager.h"

// Comms
#include "CommsChannelManager.h"
#include "ProtocolExchange.h"

// File manager
#include "FileManager.h"

// BLE
#include <BLEManager.h>

// System type consts
#include "SysTypeStatics.h"

// Hardware detection
#include "DetectHardware.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Entry Point
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// #ifndef TEST_MINIMAL_FLASH_HEAP_BASE_CASE

// Entry point
extern "C" void app_main(void)
{
    // Initialize flash
    esp_err_t flashInitResult = nvs_flash_init();
    if (flashInitResult != ESP_OK)
    {
        // Error message
        ESP_LOGE(MODULE_NAME, "nvs_flash_init() failed with error %s (%d)", esp_err_to_name(flashInitResult), flashInitResult);

        // Clear flash if possible
        if ((flashInitResult == ESP_ERR_NVS_NO_FREE_PAGES) || (flashInitResult == ESP_ERR_NVS_NEW_VERSION_FOUND))
        {
            esp_err_t flashEraseResult = nvs_flash_erase();
            if (flashEraseResult != ESP_OK)
            {
                ESP_LOGE(MODULE_NAME, "nvs_flash_erase() failed with error %s (%d)", 
                                esp_err_to_name(flashEraseResult), flashEraseResult);
            }
            flashInitResult = nvs_flash_init();
            if (flashInitResult != ESP_OK)
            {
                // Error message
                ESP_LOGW(MODULE_NAME, "nvs_flash_init() failed a second time with error %s (%d)", 
                                esp_err_to_name(flashInitResult), flashInitResult);
            }
        }
    }

    // Trace heap allocation - see RdWebConnManager.cpp and RdWebConnection.h/cpp
#ifdef DEBUG_HEAP_ALLOCATION
    heap_trace_init_standalone(trace_record, NUM_RECORDS);
#endif
    // Set hardware revision - ensure this runs early as some methods for determining
    // hardware revision may get disabled later on (e.g. GPIO pins later used for output)
    ConfigBase::setHwRevision(DetectHardware::getHWRevision());

    // Config for hardware
    ConfigBase defaultSystemConfig(defaultConfigJSON);

    // Check defaultConfigJSON is valid
    int numJsonTokens = 0;
    if (!RaftJson::validateJson(defaultConfigJSON, numJsonTokens))
    {
        LOG_E(MODULE_NAME, "mainTask defaultConfigJSON failed to parse");
    }

    ///////////////////////////////////////////////////////////////////////////////////
    // NOTE: Changing the size or order of the following will affect the layout
    // of data in the Non-Volatile-Storage partition (see partitions.csv file NVS entry)
    // This will render data in those areas invalid and this data will be lost when
    // firmware with different layout is flashed
    ///////////////////////////////////////////////////////////////////////////////////

    // Configuration for the system - including system name
    ConfigNVS _sysModMutableConfig("system", 500);

    // Configuration for system modules
    ConfigNVS _sysTypeConfig("sys", 10000);

    ///////////////////////////////////////////////////////////////////////////////////

    // SysTypes
    SysTypeManager _sysTypeManager(_sysTypeConfig);
    _sysTypeManager.setup(SYS_TYPE_STATICS, SYS_TYPE_STATICS_LEN);

    // System Module Manager
    SysManager _sysManager("SysManager", defaultSystemConfig, &_sysTypeConfig, &_sysModMutableConfig,
                    DEFAULT_FRIENDLY_NAME, SYSTEM_NAME, 
                    HW_SERIAL_NUMBER_BYTES, DEFAULT_SERIAL_SET_MAGIC_STR);

    // Add the system restart callback to the SysTypeManager
    _sysTypeManager.setSystemRestartCallback([&_sysManager] { (&_sysManager)->systemRestart(); });

    // API Endpoints
    RestAPIEndpointManager _restAPIEndpointManager;
    _sysManager.setRestAPIEndpoints(_restAPIEndpointManager);

    // Set up the hardware
    Jewelry _jewelry("Jewelry", defaultSystemConfig, &_sysTypeConfig, nullptr);

    // Comms Channel Manager
    CommsChannelManager _commsChannelManager("CommsMan", defaultSystemConfig, &_sysTypeConfig, nullptr);
    _sysManager.setCommsCore(&_commsChannelManager);

    // ProtocolExchange
    ProtocolExchange _protocolExchange("ProtExchg", defaultSystemConfig, &_sysTypeConfig, nullptr);

    // FileManager
    FileManager _fileManager("FileManager", defaultSystemConfig, &_sysTypeConfig, nullptr);
    _fileManager.setProtocolExchange(_protocolExchange);

    // Sample collector
#ifdef COLLECT_DATA_SAMPLES_WIRELESSLY
    SampleCollectorJSON _sampleCollector("HRMSamples", defaultSystemConfig, &_sysTypeConfig, nullptr);
    _sampleCollector.setSamplingInfo(0, 4000, "{\"name\":\"hrmdata\"}", "hrmsamples", true, false);

    // SerialConsole
    SerialConsole _serialConsole("SerialConsole", defaultSystemConfig, &_sysTypeConfig, nullptr);

    // NetworkManager
    NetworkManager _networkManager("NetMan", defaultSystemConfig, &_sysTypeConfig, nullptr);

    // WebServer
    WebServer _webServer("WebServer", defaultSystemConfig, &_sysTypeConfig, nullptr);

//     // ESP OTA Update
//     ESPOTAUpdate _espotaUpdate("ESPOTAUpdate", defaultSystemConfig, &_sysTypeConfig, nullptr);
    // _protocolExchange.setHandlers(&_espotaUpdate);

//     // BLEManager
//     BLEManager _bleManager("BLEMan", defaultSystemConfig, &_sysTypeConfig, nullptr, DEFAULT_ADVNAME);

//     // Command Serial
//     CommandSerial _commandSerial("CommandSerial", defaultSystemConfig, &_sysTypeConfig, nullptr);

//     // Command Socket
//     CommandSocket _commandSocket("CommandSocket", defaultSystemConfig, &_sysTypeConfig, nullptr);

//     // Command File
//     CommandFile _commandFile("CommandFile", defaultSystemConfig, &_sysTypeConfig, nullptr);

//     // MQTT
//     MQTTManager _mqttManager("MQTTMan", defaultSystemConfig, &_sysTypeConfig, nullptr);

//     // State Publisher
//     StatePublisher _statePublisher("Publish", defaultSystemConfig, &_sysTypeConfig, nullptr);

    // Log manager
    // LogManager _LogManager("LogManager", defaultSystemConfig, &_sysTypeConfig, &_sysModMutableConfig);
#endif    

    // BLEManager
    BLEManager _bleManager("BLEMan", defaultSystemConfig, &_sysTypeConfig, nullptr, DEFAULT_ADVNAME);
    
    // Log out system info
    ESP_LOGI(MODULE_NAME, SYSTEM_NAME " " SYSTEM_VERSION " (built " __DATE__ " " __TIME__ ") Heap %d", 
                        heap_caps_get_free_size(MALLOC_CAP_8BIT));

    // SysTypeManager endpoints
    _sysTypeManager.addRestAPIEndpoints(_restAPIEndpointManager);

    // Initialise the system module manager here so that API endpoints are registered
    // before file system ones
    _sysManager.setup();

#ifdef COLLECT_DATA_SAMPLES_WIRELESSLY
    // Web server add files
    String servePaths = String("/=/") + fileSystem.getDefaultFSRoot();
    servePaths += ",/files/local=/local";
    servePaths += ",/files/sd=/sd";
    _webServer.serveStaticFiles(servePaths.c_str());

    // Start webserver
    _webServer.beginServer();
#endif

    // Loop forever
    while (1)
    {
        // Yield
        vTaskDelay(1);
        // taskYIELD();

        // Service all the system modules
        _sysManager.loop();

        // Test and Monitoring
#ifdef DEBUG_SHOW_TASK_INFO
        if (Raft::isTimeout(millis(), lasttaskdumpms, DEBUG_SHOW_TASK_MS))
        {
            tasks_info();
            lasttaskdumpms = millis();
        }
#endif
#ifdef DEBUG_SHOW_RUNTIME_STATS
        if (Raft::isTimeout(millis(), laststatsdumpms, DEBUG_SHOW_RUNTIME_STATS_MS))
        {
            runtime_stats();
            laststatsdumpms = millis();
        }
#endif
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test and Monitoring
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
#include "freertos/task.h"

#ifdef DEBUG_SHOW_TASK_INFO
int tasks_info()
{
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = (char*)malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == nullptr) {
        ESP_LOGE("Main", "failed to allocate buffer for vTaskList output");
        return 1;
    }
    fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
#ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
    fputs("\tAffinity", stdout);
#endif
    fputs("\n", stdout);
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    return 0;
}
#endif

#ifdef DEBUG_SHOW_RUNTIME_STATS
int runtime_stats()
{
    const size_t bytes_per_task = 50; /* see vTaskGetRunTimeStats description */
    char *task_stats_buffer = (char*)malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_stats_buffer == nullptr) {
        ESP_LOGE("Main", "failed to allocate buffer for vTaskGetRunTimeStats output");
        return 1;
    }
    fputs("Task Name\tTime(uS)\t%", stdout);
    fputs("\n", stdout);
    vTaskGetRunTimeStats(task_stats_buffer);
    fputs(task_stats_buffer, stdout);
    free(task_stats_buffer);
    return 0;
}
#endif
#endif

#endif